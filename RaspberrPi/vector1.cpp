// cProject/vector1.cpp

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

using namespace std;

int main()
{
	vector<string> sentence;
	sentence.reserve(5);

	sentence.push_back("Hello,");
	sentence.push_back("how");
	sentence.push_back("are");
	sentence.push_back("you");
	sentence.push_back("?");
	// 输出、显示vector容器内的数据
	copy(sentence.begin(),sentence.end(),ostream_iterator<string>(cout," "));
	cout << endl;
	// 显示vector数据类型的大小
	cout << " max_size(): " << sentence.max_size() << endl;
	cout << " size(): " << sentence.size() << endl;
	cout << " capacity(): " << sentence.capacity() << endl;

	swap(sentence[1],sentence[3]);
	sentence.insert(find(sentence.begin(),sentence.end(),"?"),"always");
	sentence.back() = "!";

	copy(sentence.begin(),sentence.end(),ostream_iterator<string>(cout," "));
	cout << endl;

	cout << " max_size(): " << sentence.max_size() << endl;
	cout << " size(): " << sentence.size() << endl;
	cout << " capacity(): " << sentence.capacity() << endl;

}