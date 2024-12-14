#include <iostream>
#include <map>
#include <vector>
#include <string>
using namespace std;

int main()
{
    map<string, int> control_param = {
        { "id", 1 },
        { "name", 2 },
        { "age",3 },
        { "phone",4 },
        { "address",5 }
    };
    int id=0,name=0,age=0,phone=0,address=0;
    vector<string> vec{"id","name","age","phone","address","id","name","age","phone"};
    for (int i = 0; i < vec.size(); i++)
	{
		int caseKey = control_param[vec[i]];
        cout<<"key:"<< vec[i] <<", caseKey: "<<caseKey<<endl;
        switch (caseKey)
        {
        case 1:
            id+=1;
            cout<< "id"<<endl;
            break;
        case 2:
            name+=1;
            cout<< "name"<<endl;
            break;
        case 3:
            age+=1;
             cout<< "age"<<endl;
            break;
        case 4:
            phone+=1;
            cout<< "phone"<<endl;
            break;
        case 5:
            address+=1;
            cout<< "address"<<endl;
            break;
        default:
            break;
        }
	}
    cout<<"id:"<<id<<",name:"<<name<<",age:"<<age<<",phone:"<<phone<<",address:"<<address;
}