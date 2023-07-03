# pingme
This bot is indented to be used as a newsletter for certain channels.
You can subscribe to channels and when a new message is sent to the channel you will be notified with a DM from the bot.
For that, you have to enable private DMs from strainers.

# Usage
- `/subscribe [channel]` Subscribes you to a newsletter of a channel and notifies you when a new message appears in a given channel with a DM
- `/unsubscribe [channel]` Unsubscribes you from a newsletter for a given channel
- `/list` Lists all your subscribed newsletters

## Setup
For the bot to function you have to enable DMs from strangers, as the bot will try to send you the newsletter per DM.
To do that you have to open the _server privacy settings_ and then turn on _Direct Messages_.  
You can also enable _Allow direct messages from server members_ in the _account settings_. 
But this is **not encouraged**, as this would allow anyone on every server to DM you.  
If you don't want others to send you messages you could also add the bot to a "private" server, where only you and the bot are located.
Then you can enable _Direct Messages_ in the _server privacy settings_ for that server.

# Compile
## Dependencies
- xmake

## Compilation
To compile the project make sure you have xmake installed.
Then clone the repository, `cd` into the repository, and execute `xmake build`.
Make sure you have a token for your bot.
Add the Token as the environment variable `BOT_TOKEN`.
