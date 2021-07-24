#include "Signal.pb.h"
#include "GeneralMessageTranslator.hpp"
#include <iostream>
#include <boost/program_options.hpp>
#include <sw/redis++/redis++.h>

int main(int arguments_count, char** arguments)
{
    using namespace boost::program_options;

    options_description options("Options");

    options.add_options()
            ("help,?", "show help message.")
            ("host,h", value<std::string>(),
             "host of the Redis server.")
            ("port,p", value<unsigned int>()->default_value(6379),
             "port of the Redis server.")
            ("channel,c", value<std::string>(),
             "channel to send signal.")
            ("text,t", value<std::string>(), "text in the signal");
    variables_map variables;
    store(parse_command_line(arguments_count, arguments, options), variables);
    notify(variables);

    if (variables.count("help"))
    {
        std::cout << options << std::endl;
        return 0;
    }

    std::string host;
    if (variables.count("host"))
    {
        host = variables["host"].as<std::string>();
    } else
    {
        std::cout << "Input host:";
        std::cin >> host;
    }
    unsigned int port;
    if (variables.count("port"))
    {
        port = variables["port"].as<unsigned int>();
    } else
    {
        std::cout << "Input port:";
        std::string port_text;
        std::cin >> port_text;
        port = std::stoi(port_text);
    }
    std::string channel;
    if (variables.count("channel"))
    {
        channel = variables["channel"].as<std::string>();
    } else
    {
        std::cout << "Input channel:";
        std::cin >> channel;
    }
    std::string text;
    if (variables.count("text"))
    {
        text = variables["text"].as<std::string>();
    } else
    {
        std::cout << "Input signal text:";
        std::cin >> text;
    }
    sw::redis::Redis connection("tcp://" + host + ":" + std::to_string(port));

    Signal signal;
    signal.set_name(text);

    auto package_content = Gaia::Modules::GeneralMessageTranslator::Encode(
            3, signal.SerializeAsString());

    connection.publish(channel, package_content);

    std::cout << "Content: " << package_content << " Sent." << std::endl;

    return 0;
}