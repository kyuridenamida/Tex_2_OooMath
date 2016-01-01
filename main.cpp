#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
using namespace std;


map<string,int> num_arg;
map<string,string> tex_ooo; // [TEX] = OOo
map<string,int> isPrefixNotation;

struct NODE{
	vector<string> info; // [0] = id
	vector<NODE*> child;
	NODE(string type){
		info.push_back(type);
	}
	NODE(){}
	string id(){
		return info[0];
	}
};

class Parse{
public:
	
	Parse(string line) : line(line) {
		seek = 0; 
		line += " ";
	} //コンストラクタ
	
	char next(){ //次を確認する
		if( seek < line.size() ){
			return line[seek];
		}else{
			return -1;
		}
	}
	
	char next_ignore_space(){ // O(n) <- atode O(1) ni siyou
		for(int i = seek ; i < line.size() ; i++){
			if( line[i] != ' ' ) return line[i];
		}
		return -1;
	}
	
	void goNext(){ //1つすすめる
		seek++;
	}
	
	void goNextWhileSpace(){
		while( next() == ' ' ) goNext();
	}
	
	NODE* doParse(){
		seek = 0;
		NODE *result = formula();
		if( next_ignore_space() != -1 ){
			throw("完全なパースの失敗");
		}
		return result;
	}
	
	string getWord(){
		goNextWhileSpace();
		
		if( !~next() ) throw("パースエラー");
		string res;
		if( next() == '\\' ){
			res += next();
			goNext();
			
			if( ~next() ){ //1文字は確実に制御文字ではない
				res += next();
				goNext();
			}
			
			while( ~next() ){
				if( 'A' <= next() && next() <= 'Z' || 'a' <= next() && next() <= 'z' ){ // 英字だけ
					res += next();
					goNext();
				}else{
					break;
				}
			}
		}else{
			//ここてきとう
			if( next() == '-' || '0' <= next() && next() <= '9'){
				while( ~next() && ( next() == '-' || '0' <= next()  && next() <= '9' || next() == '.') ){
					res += next();
					goNext();
				}
			}else{
				res += next();
				goNext();
			}
			return res;
		}
	}
	
	NODE *formula(){
		NODE *node = new NODE("{}");
		goNextWhileSpace();
		while(~next() &&  next() != '}' ){
			node->child.push_back(factor());
			goNextWhileSpace();
		}
		return node;
	}

	NODE *factor(){
		goNextWhileSpace();
		if( next() == '{' ){
			goNext();
			NODE *node = formula();
			goNextWhileSpace();
			if( next() != '}' ) throw("中括弧\"{}\"エラー");
			goNext();
			return node;
		}else{
			NODE *node = new NODE(getWord());
			int n = num_arg[node->id()];
			for(int i = 0 ; i < n ; i++){
				node->child.push_back(factor());
			}
			return node;
		}
	}
	int getSeekPos(){
		return seek;
	}
	
private:
	string line;
	int seek;
};

void debug_view(NODE *root,int depth=0){
	string shift = string(depth,' ');
	if( root->id() == "{}" ){
		cout << shift + "{" << endl;
	}else{
		cout << shift + root->id() << endl;
	}
	for(int i = 0 ; i < (root->child).size() ; i++){
		debug_view(root->child[i],depth+1);
	}
	if( root->id() == "{}" ){
		cout << shift + "}" << endl;
	}
}

string output_tmp;

void toOOO(NODE *root,int depth=0){
	if( depth == 0 ) output_tmp = "";
	
	if( root->id() == "{}" ){
		output_tmp += "{";
		for(int i = 0 ; i < (root->child).size() ; i++){
			toOOO(root->child[i],depth+1);
		}
	}else{
		if( isPrefixNotation[root->id()] ){
			output_tmp += "{";
			for(int i = 0 ; i < (root->child).size() ; i++){
				if(i) output_tmp += (tex_ooo.count(root->id())?" "+tex_ooo[root->id()]+" ":root->id()); 
				toOOO(root->child[i],depth+1);	
			}
			output_tmp += "}";
		}else{
			output_tmp += (tex_ooo.count(root->id())?" "+tex_ooo[root->id()]+" ":root->id()); 
			for(int i = 0 ; i < (root->child).size() ; i++){
				toOOO(root->child[i],depth+1);	
			}
		}
	}

	if( root->id() == "{}" ){
		output_tmp += "}";
	}
}
string superfix(string t){
	vector<int> S;
	for(int i = 0 ; i < t.size() ; i++){
		if( t[i] == '(' || t[i] == ')' ){
			if( S.size() && t[S.back()] == '(' && t[i] == ')' ){
				S.pop_back();
			}else{
				S.push_back(i);
			}
		}
	}
	int offset = 0;
	string ans = "";
	
	int j = 0;
	for(int i = 0 ; i < t.size() ; i++){
		if( j < S.size() && i == S[j] ){
			j++;
			ans += "\"" + string(1,t[i]) + "\"";
		}else{
			ans += t[i];
		}
	}
	return ans;
}

string output(string s){
	string ts;
	for(int i = 0 ; i < s.size() ; i++){
		if( s[i] == '<' ){
			ts += "&lt;";
		}else if( s[i] == '>' ){
			ts += "&gt;";
		}else if( s[i] == '&' ){
			ts += "&amp;";
		}else{
			ts += s[i];
		}
	}
	return ts;
}

void doit(string formula,int lineNum=-1){
	Parse parser(formula);
	try{
		
		NODE *result = parser.doParse();
		//cout << "Tex side: " << endl;
		//debug_view(result);

		//cout << "OOo side:" << endl;
		
		toOOO(result);
		cout << output(superfix(output_tmp)) << " newline<br>";
	}catch(	const char *msg){
		cout << "<font color=\"#ff0000\">";
		cout << "エラー内容: " <<msg << "[" << lineNum << "行目の式" << "]" << "<br>" << endl;
		cout << " " << formula << "<br>" << endl;
		for(int j = 0 ; j < parser.getSeekPos() ; j++)
			cout << "&nbsp;";
		cout << "^<br>" << endl;
		cout << "</font>" << endl;
	}
}


int main(int argv,char *argc[]){
	printf("content-type: text/html\n");
	printf("\n");
	ifstream ope("op_list.txt");
	if( !ope.is_open() ){
		cerr << "op_list.txtが見つかりませんでした。" << endl;
		return 2;
	}
	string a,b,c,d;
	while(ope >> a >> b >> c >> d){
		num_arg[a] = atoi(b.c_str());
		tex_ooo[a] = c;
		//cout << a << " " << b << " " << c << " " << d << endl;
		if( d=="1"){
			
			isPrefixNotation[a] = 1;
		}
	}
	if( argv != 2 ){
		string input;
		getline(cin,input);
		if( input.size() < 5 ) return 0;
		input = input.substr(5);
		
		string parsed = "";
		for(int i = 0 ; i < input.size() ; ){
			if( input[i] == '%' ){
				parsed += ('A' <= input[i+1] ? 10 + input[i+1] - 'A' : input[i+1] - '0') * 16 + ('A' <= input[i+2] ? 10 + input[i+2] - 'A' : input[i+2] - '0');
				i += 3;
			}else{
				parsed += input[i];
				i++;
			}
		}
		stringstream sin(parsed);
		/*if( !cin.is_open() ){
			cerr << "file.txtが見つかりませんでした。" << endl;
			return 1;
		}*/
		string ln;
		int cnt = 1;
		cout << "<res>";
		while( getline(sin,ln) ){
			doit(ln,cnt++);
			
		}
		cout << "</res>";
	}else{
		string formula = string(argc[1]);
		doit(formula);
	}
	return 0;
}
