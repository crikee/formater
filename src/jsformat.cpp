#include <iostream>
#include <vector>
#include <string>
using namespace std ;

typedef vector<string> fragments ;

string 
input , indent_string , last_text , last_word , current_mode ,
whitespace[] = {
	"\n","\r","\t"
},
wordchar[] = {
	"a","b","c","d","e","f","g","h","i","j",
	"k","l","m","n","o","p","q","r","s","t",
	"u","v","w","x","y","z","A","B","C","D",
	"E","F","G","H","I","J","K","L","M","N",
	"O","P","Q","R","S","T","U","V","W","X",
	"Y","Z","0","1","2","3","4","5","6","7",
	"8","9","_","$"
} ,
punct[] = {
	"+","-","*","/","%","&","++","--","=",
	"+=","-=","*=","/=","%=","==","===",
	"!=","!==",">","<",">=","<=",">>","<<",
	">>>",">>>=",">>=","<<=","&&","&=","|",
	"||","!","!!",",",":", "?", "^", "^=" ,
	"|="
},
line_starter[] = {
	"continue","try","throw","return",
	"var","if","switch","case","default",
	"for","while","break","function"
},
else_catch_finally[] = {
	"else" , "catch" , "finally"
}
;


fragments
output ,
modes ,
wordchars	 (wordchar, wordchar+sizeof(wordchar) / sizeof(string)) ,
line_starters(line_starter,line_starter+sizeof(line_starter) / sizeof(string)),
whitespaces	 (whitespace,whitespace+sizeof(whitespace) / sizeof(string)),
puncts		 (punct,punct+sizeof(punct)/sizeof(string)),
else_catch_finallys (else_catch_finally,else_catch_finally+3);

char 
indent_char ;

unsigned int 
parser_pos , indent_size , indent_level ;

bool do_block_just_closed = false ;

enum token_type{
	TK_EOF,
	TK_WORD,
	TK_OPERATOR,
	TK_START_EXPR,
	TK_END_EXPR,
	TK_START_BLOCK,
	TK_END_BLOCK,
	TK_START_COMMAND,
	TK_END_COMMAND,
	TK_BLOCK_COMMENT,
	TK_COMMENT,
	TK_STRING,
	TK_UNKNOWN
}last_type;

struct token{
	string text ;
	token_type type ;
	token(){};
	token(string te,token_type tt):text(te),type(tt){} ;
}temp;

void trim_output(){
	unsigned int size = output.size() ; 
	while(size && (output[size-1]==" "||output[size-1]==indent_string)){
		output.pop_back();
	}
}

void print_newline(){
	unsigned int size = output.size() ; 
	trim_output();
	if(!size){
		return; // no newline on start of file
	}
	if(output[size-1]!="\n"){
		output.push_back("\n") ;
	}
	for(unsigned int i=0;i<indent_level;i++){
		output.push_back(indent_string) ;
	}	
}

void print_space(){
	unsigned int size = output.size() ; 
	string last_output = size ? output[size - 1] : " ";
	if (last_output != " "&& last_output != "\n" && last_output != indent_string) { // prevent occassional duplicate space
		output.push_back(" ");
	}
}

void print_token(){
	output.push_back(temp.text);
}

void indent(){
	indent_level++;
}


void unindent(){
	if (indent_level) {
		indent_level--;
	}
}


void remove_indent(){
	unsigned int size = output.size();
	if (size && output[size - 1] == indent_string) {
		output.pop_back();
	}
}

void set_mode(string mode){
	modes.push_back(current_mode);
	current_mode = mode ;
}

void restore_mode(){
	do_block_just_closed = (current_mode == "DO_BLOCK") ;
	modes.push_back(current_mode);
	current_mode = modes[0] ;
	modes.pop_back();
}

bool in_array(string what,fragments arr)
{
	for (unsigned int i = 0; i < arr.size(); i++)
	{
		if (arr[i] == what) {
			return true;
		}
	}
	return false;
}
    
token get_next_token(){
	unsigned int n_newlines = 0 ;
	string c = "" ;
	
	do{
		if(parser_pos >= input.size()){
			return token("",TK_EOF);
		}
		c += input[parser_pos];
		parser_pos += 1 ;
		if(c == "\n"){
			n_newlines += 1 ;
		} 
		
	}while(in_array(c,whitespaces));
	
	if(n_newlines > 1){
		for(int i=0 ; i<2 ; i++){
			print_newline();
		}
	}
	
	bool wanted_newline = (n_newlines == 1);
	
	if(in_array(c,wordchars)){
		if (parser_pos < input.size()) {
			while (in_array(""+input[parser_pos], wordchars)) {
				c += input[parser_pos];
				parser_pos += 1;
				if (parser_pos == input.size()) {
					break;
				}
			}
		}
			
		
		// small and surprisingly unugly hack for 1E-10 representation
		//if (parser_pos !== input.size() && c.match(/^[0-9]+[Ee]$/) && input[parser_pos] == '-') {
		//	parser_pos += 1;

		//	var t = get_next_token(parser_pos);
		//	c += '-' + t[0];
		//	return [c, 'TK_WORD'];
		//}

		if (c == "in") { 			// hack for 'in' operator
			return token(c, TK_OPERATOR);
		}
		return token(c, TK_WORD);
	}
	
	if(c == "(" || c == "[") 
		return token(c,TK_START_EXPR) ;
	if(c == ")" || c == "]")
		return token(c,TK_END_EXPR) ;
	if(c == "{")
		return token(c,TK_START_BLOCK);
	if(c == "}")
		return token(c,TK_END_BLOCK);
	if(c == ";")
		return token(c,TK_END_COMMAND);
	
	if(c == "/"){
		string comment = "" ;
		if(input[parser_pos] == '*'){
			parser_pos += 1 ;
			if(parser_pos < input.size()){
				if (parser_pos < input.size()) {
					while (! (input[parser_pos] == '*' && input[parser_pos + 1] && input[parser_pos + 1]== '/') && parser_pos < input.size()) {
						comment += input[parser_pos];
						parser_pos += 1;
						if (parser_pos >= input.size())
							break;				
					}
				}
				parser_pos += 2;
				return token("/*" + comment + "*/", TK_BLOCK_COMMENT);
			}
		}
		if(input[parser_pos] == '/'){
			comment += c ;
			while(input[parser_pos]!='\x0d' && input[parser_pos]!='\x0a'){
				comment += input[parser_pos] ;
				parser_pos += 1 ;
				if(parser_pos >= input.size())
					break ;
			}
			parser_pos += 1 ;
			if(wanted_newline)
				print_newline();
			return token(comment,TK_COMMENT);
		}
	}
	
	if
	(
		c == "'" || // string
		c == "\"" || // string
		(
			c == "/" &&
			(
				(last_type == TK_WORD && last_text == "return") || 
				(last_type == TK_START_EXPR || last_type == TK_END_BLOCK || 
				last_type == TK_OPERATOR || last_type == TK_EOF || last_type == TK_END_COMMAND)
			)
		)// regexp
	) { 
		char sep = c[0];
		bool esc = false;
		c = "" ;

		if (parser_pos < input.size()) {
			while (esc || input[parser_pos] != sep) {
				c += input[parser_pos];
				if (!esc) {
					esc = input[parser_pos] == '\\';
				} 
				else {
					esc = false;
				}
				parser_pos += 1;
				if (parser_pos >= input.size())
					break;
			}

		}

		parser_pos += 1;
		if (last_type == TK_END_COMMAND) {
			print_newline();
		}
		return token(sep + c + sep, TK_STRING);
	}
	
	if (in_array(c, puncts)) {
		while (parser_pos < input.size() && in_array(c + input[parser_pos], puncts)) {
			c += input[parser_pos];
			parser_pos += 1;
			if (parser_pos >= input.size())
				break;
		}
		return token(c, TK_OPERATOR) ;
	}
	return token(c, TK_UNKNOWN) ;
}



int main(){
	
	bool var_line = false , var_line_tained = false ;
	
	string current_mode = "BLOCK" ;
	string prefix = "" ;
	
	indent_level = 0 ;
	parser_pos = 0 ;
	bool in_case = false ;
	
	while(true){
		temp = get_next_token();
		string tem_te 		= temp.text ;
		token_type tem_tp 	= temp.type ;
		
		if(tem_tp == TK_EOF)
			break ;
		
		switch(tem_tp){
			
			case TK_START_EXPR : ;
				var_line = false ;
				set_mode("EXPRESSION");
				if(last_type == TK_END_EXPR || last_type == TK_START_EXPR) {
					// do nothing on (( and )( and ][ and ]( ..
				} 
				else if(last_type != TK_WORD && last_type != TK_OPERATOR){
					print_space();
				} 
				else if (in_array(last_word, line_starters) && last_word != "function"){
					print_space();
				}
				print_token();
				break;
			
			case TK_END_EXPR :
				print_token();
				restore_mode();
				break;
				
			case TK_START_BLOCK :
				if(last_word == "do")
					set_mode("DO_BLOCK");
				else
					set_mode("BLOCK");
				if(last_type != TK_OPERATOR && last_type != TK_START_EXPR){
					if(last_type == TK_START_BLOCK)
						print_newline();
					else
						print_space() ;
				}
				print_token();
				indent();
				break ;
			
			case TK_END_BLOCK :
				if(last_type == TK_START_BLOCK){
					trim_output() ;
					unindent();
				}
				else{
					unindent();
					print_newline();
				}
				print_token();
				restore_mode();
				break ;
			
			case TK_WORD :
				if(do_block_just_closed){
					print_space();
					print_token();
					print_space();
					break ;
				}
				
				if(tem_te == "case" || tem_te == "default"){
					if(last_text == ":")
						remove_indent() ;
					else{
						unindent();
						print_newline();
						indent();
					}
					print_token();
					in_case = true ;
					break ;
				}
				
				prefix = "NONE" ;
				if(last_type == TK_END_BLOCK){
					if(!in_array(tem_te , else_catch_finallys)){
					}
				}
				
				
				
			default : ;
		}
	}
	
	
	
	
	
	
	
	
}
