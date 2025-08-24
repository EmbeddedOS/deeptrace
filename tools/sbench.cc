#include <unistd.h>
#include <signal.h>

#include <log.hh>
#include <argv.hh>

class Process
{
    pid_t _pid;

public:
    Process(pid_t id) : _pid{id} {}
    Process(const std::string &bin)
    {
        pid_t pid = fork();
        if (pid == 0)
        { // Children process start from here.
        }
        else if (pid > 0)
        { // Current process continue here.
        }
        else
        { // Error case.
        }
    }

    static bool ProcessExisting(pid_t pid)
    {
        return (0 == kill(pid, 0));
    }

    static bool IsExecutable(const std::string &path){
        return access(path.c_str(), X_OK) == 0;
    }
};

class Option
{
    const char *HELP_MESSAGE = "Usage: sbench [-p pid] [-o output_file] [-h]\n"
                               "Options:\n"
                               "  -p pid          Specify the process ID to benchmark\n"
                               "  -o output_file  Specify the output file for results\n"
                               "  -h              Display this help message\n";

    pid_t _pid{0};
    bool _attach{false};
    std::string _p_name{};
    DeepTrace::Argv _p_argument{};

public:
    void Usage(int exit_code)
    {
        std::cerr << HELP_MESSAGE << std::endl;
        exit(exit_code);
    }

    Option(int argc, char *argv[])
    {
        std::string output_file{};

        if (argc < 1)
        {
            Usage(EXIT_FAILURE);
        }

        int opt = 0;
        while ((opt = getopt(argc, argv, "hp:o:")) != -1)
        {
            switch (opt)
            {
            case 'p':
                _pid = std::stoi(optarg);
                if (!Process::ProcessExisting(_pid))
                {
                    Fatal() << "PID is not running!";
                }

                _attach = true;
                break;
            case 'h':
                Usage(EXIT_SUCCESS);
                break;
            default:
                Usage(EXIT_FAILURE);
            }
        }

        if (!_attach)
        { // Brand new process.
            if (optind == argc)
            {
                Fatal() << "Please provide process name!";
            }

            _p_name = std::string(argv[optind++]);
            if (!Process::IsExecutable(_p_name))
            {
                Fatal() << "Can't execute process!";
            }

            if (optind < argc)
            {
                DeepTrace::Argv::ArgvBuilder builder{};

                for (int i = optind; i < argc; ++i)
                {
                    builder.Add(argv[i]);
                }

                _p_argument = std::move(builder);
            }
        }
        else
        { // Attach to existing process.
        }
    }
};

class Tracer
{
public:
    Tracer() = default;
    ~Tracer() = default;
};

int main(int argc, char *argv[])
{
    Option ops(argc, argv);

    return 0;
}