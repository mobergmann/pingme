#include <map>
#include <set>
#include <string>

#include <dpp/dpp.h>
#include <dpp/snowflake.h>
#include <dpp/message.h>
#include <dpp/cluster.h>

#include <fmt/core.h>


/// for each user a set of subscribed channels
std::map<dpp::snowflake, std::set<dpp::snowflake>> newsletters;

int main(int argc, char **argv)
{
    const std::string BOT_TOKEN = "MTEyNDY5NjE3Njc3NjcxNjI5OA.GALPzL.cCTMzRYQA8IHnqs4T67q5NVMTi8IJaW-NmiFHU"; //std::string(std::getenv("BOT_TOKEN"));

    dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_message_create([&](const dpp::message_create_t &event) {
        const auto origin_channel_id = event.msg.channel_id;
        const auto origin_author_id = event.msg.author.id;

        for (const auto& [subscriber_id, channels]: newsletters) {
            // if sender is the same as the iterator, then skip
            if (origin_author_id == subscriber_id) {
                return;
            }

            // check if user exists
            const auto& subscriber = dpp::find_user(subscriber_id);
            if (not subscriber) {
                // delete user which does not exit
                newsletters.erase(subscriber_id);
                return;
            }

            // if user has a newsletters
            if (not channels.contains(origin_channel_id)) {
                return;
            }

            // try to get channel
            const auto channel = dpp::find_channel(origin_channel_id);
            if (not channel) {
                // delete channel which does not exit
                newsletters[subscriber_id].erase(origin_channel_id);
                return;
            }

            // ping user, that a new message was received
            dpp::message m(fmt::format("A new message in the channel {} was send. Go check it out.", channel->get_mention()));
            bot.direct_message_create(subscriber_id, m, [](const auto& e){
                // todo here a dm couldn't be delivered to the user. the user has to enable messages from stringers.
            });
        }
    });

    bot.on_slashcommand([&bot](const dpp::slashcommand_t &event) {
        if (event.command.get_command_name() == "list") {
            const auto& channels = newsletters[event.command.usr.id];
            if (channels.empty()) {
                event.reply("You have no active subscriptions.");
            }
            else {
                std::string reply = "You subscribed for the newsletters of the following channels: ";
                for (const auto& channel_id: channels) {
                    const auto channel = dpp::find_channel(channel_id);
                    if (not channel) {
                        newsletters[event.command.usr.id].erase(channel_id);
                        reply += "\n- This channel does not exist anymore. It will be removed from the list.";
                    } else {
                        // todo: if channel is category
                        reply += "\n- " + channel->get_mention();
                    }
                }
                event.reply(reply);
            }
        }
        else if (event.command.get_command_name() == "subscribe") {
            dpp::snowflake channel_id = std::get<dpp::snowflake>(event.get_parameter("channel"));
            const auto channel = dpp::find_channel(channel_id);
            // todo only continue if bot has access to channel
            if (not channel) {
                event.reply("The channel you provided does not exist.");
            } else {
                newsletters[event.command.usr.id].insert(channel_id);
                event.reply(fmt::format("You subscribed for the newsletters for the channel {}", channel->get_mention()));
            }
        }
        else if (event.command.get_command_name() == "unsubscribe") {
            dpp::snowflake channel_id = std::get<dpp::snowflake>(event.get_parameter("channel"));
            newsletters[event.command.usr.id].erase(channel_id);
            const auto channel = dpp::find_channel(channel_id);
            if (not channel) {
                event.reply("The channel you provided does not exist.");
            } else {
                event.reply(fmt::format("You have successfully removed the channel {} from your newsletters.", channel->get_mention()));
            }
        }
    });

    bot.on_ready([&bot](const dpp::ready_t &event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            dpp::slashcommand list("list", "Lists all your subscribed channels.", bot.me.id);
            dpp::slashcommand subscribe("subscribe", "Notifies you when a new message appears in a given channel with a DM.", bot.me.id);
            dpp::slashcommand unsubscribe("unsubscribe", "Unsubscribes you from a newsletters for a given channel.", bot.me.id);

            subscribe.add_option(
                dpp::command_option(dpp::co_channel, "channel", "Id of the channel you want to subscribe to.", true)
            );
            unsubscribe.add_option(
                dpp::command_option(dpp::co_channel, "channel", "Id of the channel you want to unsubscribe from.", true)
            );

            bot.global_command_create(list);
            bot.global_command_create(subscribe);
            bot.global_command_create(unsubscribe);
        }
    });
  
    bot.start(dpp::st_wait);

    return 0;
}
