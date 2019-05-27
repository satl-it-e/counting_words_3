std::ifstream in_f(mc.in_file);
if (! in_f)
{ std::cerr << "Couldn't open input-file."; return -2; }

std::string content; std::stringstream ss;
ss << in_f.rdbuf();
content = ss.str();



// ~~~~ args ~~~~
std::string cnf_name;
if(argc > 1){
cnf_name = argv[1];
} else { cnf_name = "config.dat"; }

if (!is_file_ext(cnf_name, ".txt") && !is_file_ext(cnf_name, ".dat")) {
std::cerr << "Wrong config file extension." << std::endl;
return -1;
}


// ~~~~ config ~~~~
MyConfig mc;
mc.load_configs_from_file(cnf_name);

if (mc.is_configured()) {
std::cout << "YES! Configurations loaded successfully.\n" << std::endl;
} else {
std::cerr << "Error. Not all configurations were loaded properly.";
return -3;
}


// ~~~~ test ~~~~
class HelloWorldTask : public QRunnable
{
    void run() override
    {
        qDebug() << "Hello world from thread" << QThread::currentThread();
    }
};

auto *hello = new HelloWorldTask();
auto thread_pool = QThreadPool::globalInstance();
thread_pool->start(hello);


//    ->start(hello);


