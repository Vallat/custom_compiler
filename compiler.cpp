#include "compiler.h"

#include "lexeme_analyzer.cpp"
#include "parser.cpp"
#include "translator.cpp"

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Input the code file name and the name of the file you want to put translated code in\n";
		return 0;
	}

	std::ifstream input_code_file(std::move(argv[1]));
	std::fstream lexemes_table_file("lexemes_table.txt", std::ios::trunc | std::ios::in | std::ios::out);

	c_lexeme_analyzer analyzer(&input_code_file, &lexemes_table_file);
	analyzer.do_lexeme_analyze();

	std::fstream rules_chain("rules_chain.txt", std::ios::trunc | std::ios::in | std::ios::out);
	std::fstream tokens_file("tokens.txt", std::ios::trunc | std::ios::in | std::ios::out);
	parser current_parser(&lexemes_table_file, &rules_chain, &tokens_file);

	current_parser.do_parsing();

	std::ofstream writing_file(std::move(argv[2]), std::ios::trunc | std::ios::out);

	translator current_translator(&writing_file, &rules_chain, &tokens_file);
	current_translator.translate();

	input_code_file.close();
	lexemes_table_file.close();
	rules_chain.close();
	tokens_file.close();
	writing_file.close();

	remove("lexemes_table.txt");
	remove("rules_chain.txt");
	remove("tokens.txt");

	std::cout << "The translation was successful";
	return 0;
}