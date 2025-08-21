#pragma once
#include <string>
#include <vector>

namespace DTrace
{
    class Arg
    {
        std::vector<std::string> _args;

    private:
        void Push(const std::string &arg)
        {
            _args.push_back(arg);
        }

    public:
        class Argv
        {
            int _len;
            char **_buffer;

        public:
            operator const char **()
            {
                return (const char **)_buffer;
            }

            Argv(Argv &other) = delete;
            Argv() = delete;
            Argv &operator=(Argv &other) = delete;
            Argv &operator=(Argv &&other) = delete;

            Argv(Argv &&other)
            {
                _len = other._len;
                _buffer = other._buffer;

                other._len = 0;
                other._buffer = nullptr;
            }

            Argv(const Arg &args)
            {
                if (!args._args.size())
                {
                    return;
                }

                _len = args._args.size() + 1;
                _buffer = new char *[_len];

                for (std::size_t i = 0; i < args._args.size(); i++)
                {
                    if (!args._args[i].size())
                    {
                        continue;
                    }

                    _buffer[i] = new char[args._args[i].size()];
                    args._args[i].copy(_buffer[i], args._args[i].size());
                }

                _buffer[_len - 1] = nullptr;
            }

            ~Argv()
            {
                if (_len == 0 || !_buffer)
                {
                    return;
                }

                for (int i = 0; i < _len; i++)
                {
                    delete[] _buffer[i];
                }

                delete[] _buffer;
            }
        };

        Arg(const std::string &arg)
        {
            Push(arg);
        }

        template <typename... Args>
        Arg(const std::string &head, Args... args) : Arg(args...)
        {
            Push(head);
        }

        Arg &Add(const std::string &arg)
        {
            Push(arg);
            return *this;
        }

        Argv Finalize()
        {
            return Argv{*this};
        }

        ~Arg() = default;
    };
}