// this is demo program_options
// g++ po.cpp -o po -lboost_program_options

#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>
namespace po = boost::program_options;

void print_vm(po::options_description& opts, po::variables_map& vm)
{
	if(vm.empty())
	{
		std::cout << opts << std::endl;
	}
	if(vm.count("help"))
	{
		std::cout << opts << std::endl;
	}

	std::cout << "find opt:" << vm["filename"].as<std::string>() << std::endl;
	
	if(vm.count("dir"))
	{
		std::cout << "dir opt:";
		for(auto& str : vm["dir"].as<std::vector<std::string> >())
		{
			std::cout << str << ",";
		}
		std::cout << std::endl;
	}
	if(vm.count("depth"))
	{
		std::cout << "depth opt:" << vm["depth"].as<int>() << std::endl;
	}
		
	
}
int main(int argc, char* argv[])
{
	po::options_description opts("demo options");
	std::string filename;
	opts.add_options()
		("help,h", "help message\n    a bit of long text")
		("filename,f", po::value<std::string>(&filename)->default_value("test"), "to find a file")
		("dir,D",po::value<std::vector<std::string> >()->multitoken(),"search dir")
		("depth,d",po::value<int>()->implicit_value(5),"search depth");


	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, opts), vm);
	po::notify(vm);

	// 解析完成，调用函数实现选项处理逻辑
	print_vm(opts, vm);
	
	return 0;
}
