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
        if (event.command.get_command_name() == "newsletter") {
            std::string channel = std::get<std::string>(event.get_parameter("channel"));
            newsletter[event.command.user.id].insert(channel);
            event.reply(fmt::format("You applied for the newsletter for the channel {}", channel));
            // todo test if bot has access to channel
        }
        else if (event.command.get_command_name() == "list") {
            event.reply(fmt::format("You applied for the newsletter of the following channels: {}", [event.command.user.id]));
        }
        else if (event.command.get_command_name() == "unsubscribe") {
            newsletter.remove(event.command.user.id);
        }
    });

    bot.on_ready([&bot](const dpp::ready_t &event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            dpp::slashcommand scan("newsletter", "Notifies you when a new message appears in the given channel.", bot.me.id);

            dpp::slashcommand count("profile", "counts the reactions of the calling user and returns the results", bot.me.id);
            count.add_option(
                dpp::command_option(dpp::co_string, "level", "the level of the search for messages. "
//                                                             "if the level is local, only the current channel is searched. "
//                                                             "if the level is guild, then all channels in the current guild are being searched. *1 ",
//                                                             "if the level is global, then all guilds are being searched. *1 ",
//                                                             "*1 Keep in mind, that only channels are scanned, which are indexed, aka channels in which /scan has been called.",
                                    , false)
                    .add_choice(dpp::command_option_choice("local", std::string("local")))
                    .add_choice(dpp::command_option_choice("guild", std::string("guild")))
                    .add_choice(dpp::command_option_choice("global", std::string("global")))
            );

            bot.global_command_create(scan);
            bot.global_command_create(count);
        }
    });
  
    bot.start(dpp::st_wait);

    return 0;
}
