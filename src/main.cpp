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

    bot.on_slashcommand([&bot](const dpp::slashcommand_t &event) {
        if (event.command.get_command_name() == "list") {
            const auto& channels = newsletter[event.command.usr.id];
            if (channels.empty()) {
                event.reply("You have no active subscriptions.");
            }
            else {
                std::string reply = "You subscribed for the newsletter of the following channels: ";
                for (const auto& i: channels) {
                    reply += "\n - " + std::to_string(i);
                }
                event.reply(reply);
            }
        }
        else if (event.command.get_command_name() == "subscribe") {
            dpp::snowflake channel = std::get<dpp::snowflake>(event.get_parameter("channel"));
            event.reply(std::to_string(channel));
//            // todo test if bot has access to channel
//            newsletter[event.command.usr.id].insert(channel);
//            event.reply(fmt::format("You subscribed for the newsletter for the channel {}", channel));
        }
        else if (event.command.get_command_name() == "unsubscribe") {
            dpp::snowflake channel_id = std::get<dpp::snowflake>(event.get_parameter("channel"));
            newsletter[event.command.usr.id].erase(channel_id);
            event.reply(fmt::format("You have successfully removed the channel {} from your newsletter", std::to_string(channel_id)));
        }
    });

    bot.on_ready([&bot](const dpp::ready_t &event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            dpp::slashcommand list("list", "Lists all your subscribed channels.", bot.me.id);
            dpp::slashcommand subscribe("subscribe", "Notifies you when a new message appears in the given channel.", bot.me.id);
            dpp::slashcommand unsubscribe("unsubscribe", "unsubscribes you from a newsletter for a given channel.", bot.me.id);

            subscribe.add_option(
                dpp::command_option(dpp::co_channel, "channel_id", "Id of the channel you want to watch.", true)
            );
            unsubscribe.add_option(
                dpp::command_option(dpp::co_channel, "channel_id", "Id of the channel you want to watch.", true)
            );

            bot.global_command_create(list);
            bot.global_command_create(subscribe);
            bot.global_command_create(unsubscribe);
        }
    });
  
    bot.start(dpp::st_wait);

    return 0;
}
