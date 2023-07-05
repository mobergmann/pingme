#include <map>
#include <set>
#include <string>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <dpp/dpp.h>
#include <dpp/snowflake.h>
#include <dpp/message.h>
#include <dpp/cluster.h>

#include <fmt/core.h>

#include <yaml-cpp/yaml.h>


/// for each user a set of subscribed channels
std::map<dpp::snowflake, std::set<dpp::snowflake>> newsletters;
/// mutex for locking the newsletters global variable
std::mutex lock;

const std::filesystem::path newsletters_file_path = "newsletters.yaml";

void deserialize() {
    if (not std::filesystem::exists(newsletters_file_path)) {
        std::cerr << "the file '" << newsletters_file_path << "' could not be found. this means, that the bot will start with empty subscriptions" << std::endl;
        return;
    }

    lock.lock();

    YAML::Node map = YAML::LoadFile(newsletters_file_path);
    for (const auto k: map) {
        const auto key = k.first.as<uint64_t>();

        std::set<dpp::snowflake> value;
        for (const auto v: map[key]) {
            const auto x = v.as<uint64_t>();
            value.insert(dpp::snowflake(x));
        }

        newsletters[dpp::snowflake(key)] = value;
    }

    lock.unlock();
}

void serialize() {
    lock.lock();

    YAML::Emitter emitter;
    emitter << newsletters;

    // write json to file
    std::ofstream o(newsletters_file_path);
    o << emitter.c_str() << std::endl;

    lock.unlock();
}

void serialize_signal(int signal) {
    serialize();
}

void list(const dpp::slashcommand_t &event) {
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
    serialize();
}

void subscribe(dpp::cluster &bot, const dpp::slashcommand_t &event) {
    lock.lock();

    dpp::snowflake channel_id = std::get<dpp::snowflake>(event.get_parameter("channel"));
    const auto channel = dpp::find_channel(channel_id);
    // todo only continue if bot has access to channel
    if (not channel) {
        lock.unlock();
        event.reply("The channel you provided does not exist.");
        return;
    }

    // test if the bot can send a dm to the user
    dpp::message m(fmt::format("I will send you the newsletter for the channel {} you subscribed to per DM.", channel->get_mention()));
    bot.direct_message_create(event.command.usr.id, m, [event, channel, channel_id](const auto& e){
        if (e.is_error()) {
            lock.unlock();
            event.reply("It seems, that I couldn't send you a DM. Please enable the server setting 'Privacy Settings'->'Direct Messages=ON'");
            return;
        }

        newsletters[event.command.usr.id].insert(channel_id);
        event.reply(fmt::format("You subscribed for the newsletters for the channel {}", channel->get_mention()));

        lock.unlock();
    });
}

void unsubscribe(const dpp::slashcommand_t &event) {
    lock.lock();

    dpp::snowflake channel_id = std::get<dpp::snowflake>(event.get_parameter("channel"));

    newsletters[event.command.usr.id].erase(channel_id);

    const auto channel = dpp::find_channel(channel_id);
    if (not channel) {
        event.reply("The channel you provided does not exist.");
    } else {
        event.reply(fmt::format("You have successfully removed the channel {} from your newsletters.", channel->get_mention()));
    }

    lock.unlock();

    serialize();
}

void handle_message(dpp::cluster &bot, const dpp::message_create_t &event) {
    const auto origin_channel_id = event.msg.channel_id;
    const auto origin_author_id = event.msg.author.id;

    lock.lock();

    for (const auto& [subscriber_id, channels]: newsletters) {
        // if sender is the same as the iterator, then skip
        if (origin_author_id == subscriber_id or origin_author_id == bot.me.id) {
            lock.unlock();
            return;
        }

        // check if user exists
        const auto& subscriber = dpp::find_user(subscriber_id);
        if (not subscriber) {
            // delete user which does not exit
            newsletters.erase(subscriber_id);
            lock.unlock();
            return;
        }

        // if user has a newsletters
        if (not channels.contains(origin_channel_id)) {
            lock.unlock();
            return;
        }

        // try to get channel
        const auto channel = dpp::find_channel(origin_channel_id);
        if (not channel) {
            // delete channel which does not exit
            newsletters[subscriber_id].erase(origin_channel_id);
            lock.unlock();
            return;
        }

        // ping user, that a new message was received
        dpp::message m(fmt::format("A new message in the channel {} was send. Go check it out.", channel->get_mention()));
        bot.direct_message_create(subscriber_id, m, [event](const auto& e){
            /*if (e.is_error()) {
                event.reply("It seems, that I couldn't send you a DM. Please enable the server setting 'Privacy Settings'->'Direct Messages=ON'");
                return;
            }*/
            // todo: what to do when we cannot ping a user anymore (maybe, because he/she turned of the privacy feature)?
            lock.unlock();

            serialize();
        });
    }
}

int main(int argc, char **argv)
{
    /* todo
    // when an interrupt signal occurs serialize the newsletters
    std::signal(SIGINT, &serialize_signal);
    // when a terminate signal occurs serialize the newsletters
    std::signal(SIGTERM, &serialize_signal);
    */

    // initially deserialize the json file to resume the last session
    deserialize();

    const char* BOT_TOKEN_ptr = std::getenv("BOT_TOKEN");
    if (not BOT_TOKEN_ptr) {
        std::cerr << "Provide a environment variable with the name 'BOT_TOKEN'" << std::endl;
        std::exit(1);
    }
    const std::string BOT_TOKEN = std::string(BOT_TOKEN_ptr);

    dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_message_create([&](const dpp::message_create_t &event) {
        handle_message(bot, event);
    });

    bot.on_slashcommand([&bot](const dpp::slashcommand_t &event) {
        if (event.command.get_command_name() == "list") {
            list(event);
        }
        else if (event.command.get_command_name() == "subscribe") {
            subscribe(bot, event);
        }
        else if (event.command.get_command_name() == "unsubscribe") {
            unsubscribe(event);
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
