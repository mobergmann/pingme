#include <map>
#include <set>
#include <string>

#include <dpp/dpp.h>
#include <dpp/snowflake.h>
#include <dpp/message.h>
#include <dpp/cluster.h>

#include <fmt/core.h>

std::map<dpp::snowflake, std::set<dpp::snowflake>> newsletter;

int main(int argc, char **argv)
{
    const std::string BOT_TOKEN = "MTEyNDY5NjE3Njc3NjcxNjI5OA.GALPzL.cCTMzRYQA8IHnqs4T67q5NVMTi8IJaW-NmiFHU"; //std::string(std::getenv("BOT_TOKEN"));

    dpp::cluster bot(BOT_TOKEN);

    bot.on_log(dpp::utility::cout_logger());

    // bot.onmessage_send(); // todo

    bot.on_slashcommand([&bot](const dpp::slashcommand_t &event) {
        if (event.command.get_command_name() == "list") {
            const auto& channels = newsletter[event.command.usr.id];
            if (channels.empty()) {
                event.reply("You have no active subscriptions.");
            }
            else {
                std::string reply = "You subscribed for the newsletter of the following channels: ";
                for (const auto& channel_id: channels) {
                    const auto channel = dpp::find_channel(channel_id);
                    if (not channel) {
                        newsletter[event.command.usr.id].erase(channel_id);
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
            // todo test if bot has access to channel
            if (not channel) {
                event.reply("The channel you provided does not exist.");
            } else {
                newsletter[event.command.usr.id].insert(channel_id);
                event.reply(fmt::format("You subscribed for the newsletter for the channel {}", channel->get_mention()));
            }
        }
        else if (event.command.get_command_name() == "unsubscribe") {
            dpp::snowflake channel_id = std::get<dpp::snowflake>(event.get_parameter("channel"));
            newsletter[event.command.usr.id].erase(channel_id);
            const auto channel = dpp::find_channel(channel_id);
            if (not channel) {
                event.reply("The channel you provided does not exist.");
            } else {
                event.reply(fmt::format("You have successfully removed the channel {} from your newsletter.", channel->get_mention()));
            }
        }
    });

    bot.on_ready([&bot](const dpp::ready_t &event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            dpp::slashcommand list("list", "Lists all your subscribed channels.", bot.me.id);
            dpp::slashcommand subscribe("subscribe", "Notifies you when a new message appears in a given channel with a DM.", bot.me.id);
            dpp::slashcommand unsubscribe("unsubscribe", "Unsubscribes you from a newsletter for a given channel.", bot.me.id);

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
