




#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <cctype>
#include <algorithm>

void ltrim(std::string & line)
{
	auto nospace = [](char ch) { return not std::isspace(ch); };
	auto it = std::find_if(line.begin(), line.end(), nospace);
	line.erase(line.begin(), it);
}

void rtrim(std::string & line)
{
	int i=line.size()-1;
	for( ; i>=0 ; --i)
	{
		if(not std::isspace(line[i])) break;
	}
	line.erase(i+1,line.size()-1);
}

void trim( std::string & line )
{
		ltrim(line);
		rtrim(line);
}
int skip(std::string & line,int &i)
{
	for( ; i<line.size() ; ++i)
	{
		if(line[i]!=' ')
			break;
	}
	return i-1;
}


std::vector<std::string>  tokenize(std::string & line)
{
	if(line.back()!=';')
		throw std::runtime_error(" missing ;");
	std::vector<std::string> tokens;
	std::string str="";

	for(int i=0 ; i<line.size() ; ++i)
	{
		switch(line[i])
		{
			case ' ':
				if(not str.empty())
					tokens.push_back(str);
				str="";
				i++;
				i=skip(line,i);
				break;
			case '(':
				if(not str.empty())
					tokens.push_back(str);
				tokens.push_back("(");
				str="";
				break;
			case ')':
				if(not str.empty())
					tokens.push_back(str);
				tokens.push_back(")");
				str="";
				break;
			case '=':
				if(not str.empty())
					 tokens.push_back(str);
				tokens.push_back("=");
				str="";
				break;
			case '"':
				{
					int j=i;
					for(++i; i<line.size() ; ++i)
					{
						if(line[i]=='"')
						{
							break;		
						}
					}
					str=line.substr(j,i-j+1);
					tokens.push_back(str);
					str="";
					break;
				}
			default:
				str+=line[i];
		}
	}
	return tokens;
}

bool is_print_stmt(std::vector<std::string> const & tokens)
{
	if(tokens.size()!=4)
	{
		return false;
	}
	
	return ((tokens[0] =="print" or tokens[0] =="println")
			and tokens[1] =="(" 
			and tokens[3] ==")");
}
bool is_scan_stmt(std::vector<std::string> const & tokens)
{
	if(tokens.size()!=6)
		return false;

	return tokens[1] == "="
		and tokens[2] =="scan"
		and tokens[3] =="("
		and tokens[5] ==")";
}

bool print(std::vector<std::string> const & tokens,std::vector<std::string> const & variables,std::ofstream & out)
{
	if(not is_print_stmt(tokens))
		return false;

	out<<"std::cout<<";
	if(tokens[2][0]=='"')
		out<<tokens[2];
	else
	{
		if (std::find(variables.begin(), variables.end(), tokens[2]) == variables.end())
			throw std::runtime_error("variable not declared above");
		out << tokens[2];
	}
	if(tokens[0]=="println") out<<"<<std::endl;\n";
	else out<<";\n";
	return true;

}

bool scan(std::vector<std::string> const & tokens,std::vector<std::string> & variables,std::ofstream & out)
{
	if( not is_scan_stmt(tokens))
		return false;

	variables.push_back(tokens[0]);

	if(tokens[4]=="string")
		out<<"std::"<<tokens[4]<<" ";
	else if(tokens[4]=="int")
		out<<tokens[4]<<" ";
	else if(tokens[4]=="float")
		out<<tokens[4]<<" ";
	else
		throw std::runtime_error("type mismatch error occured ");
	
	for(int i=0 ;i<tokens[0].size() ; ++i)
	{
		if(tokens[0][i]=='"' and tokens[0][i] >='0' and tokens[0][i] <='9')
			throw std::runtime_error("variable name shouldn't initialize with [0-9]");
	}
	out<<tokens[0]<<";\n";
	out<<"std::cin>>"<<tokens[0]<<";\n";
	return true;
}

bool comment(std::string const & line,std::ofstream & out)
{
	if(line[0]!='#')
		return false;
	out<<"//";
	//for(int i=1 ; i<line.size() ; ++i)
	//	out<<line[i];
	out.write(line.data() + 1, line.size() - 1); 
	out<<"\n";
	return true;
}

int main()
{
	std::ifstream in("prog.mylang");
	std::ofstream out("generatedd.cpp");
	out<<"#include <iostream>\n#include <string>\n\n\nint main()\n{\n";

	int counter=1;
	try
	{
		std::vector<std::string> variables;
		std::string line;
		while(std::getline(in,line))
		{
			if(comment(line,out))
				continue;
			
			trim(line);

			if(line.empty()) 
				continue;

			std::vector<std::string> tokens= tokenize(line);
			
			if(not (print(tokens,variables,out) or scan(tokens,variables,out)) ) 
				throw std::runtime_error("syntax error");
			++counter;
		}
		out<<"}\n";
	}
	catch(std::exception const & e)
	{
		std::cout<<"error expected at line: "<<counter<<e.what()<<std::endl;
	}
}
