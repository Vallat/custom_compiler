#include "compiler.h"

class parser
{
private:

	bool reading_in_progress = false;
	size_t input_chain_size = 0;

	std::fstream* input_file;
	std::fstream* output_file;
	std::fstream* tokens_file;

	std::string input_chain[INPUT_CHAIN_MAX_LENGHT];

	std::string grammar_rules[RULES_AMOUNT] = { "E;E", "E;", "{E}", "ifEthenEelseE", "ifEthenE", "a=E", "EorE", "EandE", "notE", "E<E", "E>E", "E==E", "E+E", "E-E", "E/E", "E*E", "E=E", "(E)", "a", "c" };

	std::string lexem_names_to_ignore[IGNORABLE_AMOUNT] = { "-", "Lexeme", "Comment", "Identificator", "Assign", "BinaryConstant", "Composite", "Logical", "Compare", "\"then\"", "\"else\"", "\"and\"", "\"not\"", "\"or\"", "\"if\"" };
	std::string possible_symbols[SYMBOLS_AMOUNT] = { "if", "else", "then", "and", "not", "or", "=", "<", ">", "==", "+", "-", "*", "/", "(", ")", "{", "}", ";", "a", "c"};
	char rules_matrix[SYMBOLS_AMOUNT][SYMBOLS_AMOUNT] =

	//	if		else	then	and		not		or		=		<		>		==		+		-		*		/		(		)		{		}		;		a		c		⊥к ($)
	{
		' ',	' ',	'=',	'<',	'<',	'<',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	' ',	' ',	' ',	' ',	'<',	'<',	' ',	// if
		'<',	'>',	' ',	'<',	'<',	'<',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	' ',	'<',	'>',	'>',	'<',	'<',	'>',	// else
		'<',	'=',	' ',	'<',	'<',	'<',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	' ',	'<',	'>',	'>',	'<',	'<',	'>',	// then
		' ',	'>',	'>',	'>',	'<',	'>',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'>',	' ',	'>',	'>',	'<',	'<',	'>',	// and
		' ',	'>',	'>',	'>',	'<',	'>',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'>',	' ',	'>',	'>',	'<',	'<',	'>',	// not
		' ',	'>',	'>',	'>',	'<',	'>',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'>',	' ',	'>',	'>',	'<',	'<',	'>',	// or
		' ',	'>',	' ',	'>',	'<',	'<',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	' ',	' ',	'>',	'>',	'<',	'<',	'>',	// =
		' ',	'>',	'>',	'>',	' ',	'>',	' ',	' ',	' ',	' ',	'<',	'<',	'<',	'<',	'<',	'>',	' ',	'>',	'>',	'<',	'<',	'>',	// <
		' ',	'>',	'>',	'>',	' ',	'>',	' ',	' ',	' ',	' ',	'<',	'<',	'<',	'<',	'<',	'>',	' ',	'>',	'>',	'<',	'<',	'>',	// >
		' ',	'>',	'>',	'>',	' ',	'>',	' ',	' ',	' ',	' ',	'<',	'<',	'<',	'<',	'<',	'>',	' ',	'>',	'>',	'<',	'<',	'>',	// ==
		' ',	'>',	'>',	'>',	' ',	'>',	' ',	'>',	'>',	'>',	'>',	'>',	'>',	'>',	'<',	'>',	' ',	'>',	'>',	'<',	'<',	'>',	// +
		' ',	'>',	'>',	'>',	' ',	'>',	' ',	'>',	'>',	'>',	'>',	'>',	'>',	'>',	'<',	'>',	' ',	'>',	'>',	'<',	'<',	'>',	// -
		' ',	'>',	'>',	'>',	' ',	'>',	' ',	'>',	'>',	'>',	'>',	'>',	'>',	'>',	'<',	'>',	' ',	'>',	'>',	'<',	'<',	'>',	// *
		' ',	'>',	'>',	'>',	' ',	'>',	' ',	'>',	'>',	'>',	'>',	'>',	'>',	'>',	'<',	'>',	' ',	'>',	'>',	'<',	'<',	'>',	// /
		' ',	' ',	' ',	'<',	'<',	'<',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'=',	' ',	' ',	' ',	'<',	'<',	' ',	// (
		' ',	'>',	'>',	'>',	' ',	'>',	' ',	'>',	'>',	'>',	'>',	'>',	'>',	'>',	' ',	'>',	' ',	'>',	'>',	' ',	' ',	'>',	// )
		'<',	'>',	'>',	'<',	'<',	'<',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	' ',	'<',	'=',	'<',	'<',	'<',	' ',	// {
		' ',	'>',	'>',	' ',	' ',	' ',	' ',	' ',	' ',	' ',	' ',	' ',	' ',	' ',	' ',	' ',	' ',	'>',	' ',	' ',	' ',	'>',	// }
		'<',	' ',	' ',	'<',	'<',	'<',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	' ',	'<',	'>',	'>',	'<',	'<',	'>',	// ;
		' ',	'>',	'>',	'>',	' ',	'>',	'=',	'>',	'>',	'>',	'>',	'>',	'>',	'>',	' ',	'>',	' ',	'>',	'>',	' ',	' ',	'>',	// a
		' ',	'>',	'>',	'>',	' ',	'>',	' ',	'>',	'>',	'>',	'>',	'>',	'>',	'>',	' ',	'>',	' ',	'>',	'>',	' ',	' ',	'>',	// c
		'<',	' ',	' ',	'<',	'<',	'<',	' ',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	'<',	' ',	'<',	' ',	'<',	'<',	'<',	' ',	// ⊥н (#)

	};

	void write_to_log(std::string what_to_print)
	{
		*output_file << what_to_print;
	}

	void write_token(std::string token)
	{
		*tokens_file << token << std::endl;
	}

	int get_relation(std::string left, std::string right)
	{
		int row = -1, column = -1;
		for (size_t iterator = 0; iterator < SYMBOLS_AMOUNT-1; iterator++)
		{
			if (row > 0 && column > 0) break;
			std::string current_symbol = possible_symbols[iterator];
			if (row < 0 && !left.compare(current_symbol)) row = iterator;
			if (column < 0 && !right.compare(current_symbol)) column = iterator;
		}
		if (row < 0)
		{
			if (!left.compare("$") || !left.compare("#")) row = 21;
			else
			{
				reading_in_progress = false;
				return 0;
			}
		}
		if (column < 0)
		{
			if (!right.compare("$") || !right.compare("#")) column = 21;
			else
			{
				reading_in_progress = false;
				return 0;
			}
		}
		return (int)rules_matrix[row][column];
	}

	int get_rule_number(std::string rule_to_search)
	{
		for (size_t iterator = 0; iterator < RULES_AMOUNT; iterator++)
		{
			std::string current_symbol = grammar_rules[iterator];
			if (!current_symbol.compare(rule_to_search))
			{
				return (int)(iterator + 1);
			}
		}
		return 0;
	}

	void parse_input_chain()
	{
		std::string reading_line, reading_word;
		while (!input_file->eof())
		{
			std::getline(*input_file, reading_line);
			if (reading_line.find("Comment") != std::string::npos) continue;
			
			if (reading_line.find("Identificator") != std::string::npos)
			{
				write_token(reading_line.substr(0, reading_line.find(' ')));
				reading_word = "a";
			}
			else if (reading_line.find("BinaryConstant") != std::string::npos)
			{
				write_token(reading_line.substr(0, reading_line.find(' ')));
				reading_word = "c";
			}
			else
			{
				reading_word = reading_line.substr(0, reading_line.find(' '));
			}
			
			input_chain[input_chain_size++] = reading_word;
		}
		input_chain[input_chain_size > 0 ? input_chain_size - 1 : 0] = "#";
	}

	std::string find_terminal_symbol_in_range(const std::string *magazine_memory, size_t to_range)
	{
		while (!magazine_memory[to_range].compare("E"))
		{
			to_range--;
		}
		return magazine_memory[to_range];
	}

	void parse()
	{
		reading_in_progress = true;
		size_t reading_head_position = 0, line_counter = 0, magazine_current_size = 0;
		std::string magazine_memory[MAGAZINE_MEMORY_AMOUNT], last_terminal_symbol, rules_string = "Rules chain: ";
		magazine_memory[0] = "$";

		while (reading_in_progress)
		{
			last_terminal_symbol = find_terminal_symbol_in_range(magazine_memory, magazine_current_size);

			if (!input_chain[reading_head_position].compare("#") && !last_terminal_symbol.compare("$"))
			{
				write_to_log(rules_string);
				reading_in_progress = false;
				break;
			}

			int current_relation = get_relation(last_terminal_symbol, input_chain[reading_head_position]);
			switch (current_relation)
			{
			case '<': case '=':
				magazine_current_size++;
				magazine_memory[magazine_current_size] = input_chain[reading_head_position];
				reading_head_position++;
				break;

			case '>':
			{
				std::string previous_terminal_symbol, rule_string;
				std::string cached_last_terminal_symbol = last_terminal_symbol;		//we need it for handling rule "ifEthenEelseE"
				size_t last_terminal_position = magazine_current_size;

				while (magazine_memory[last_terminal_position] != last_terminal_symbol) last_terminal_position--;
				
				size_t base = last_terminal_position, base_count = 0;
				while (true)
				{
					base--;
					previous_terminal_symbol = find_terminal_symbol_in_range(magazine_memory, base);
					while (magazine_memory[base] != previous_terminal_symbol) base--;

					size_t relations_of_previous_terminal_symbols = get_relation(previous_terminal_symbol, cached_last_terminal_symbol);
					if (relations_of_previous_terminal_symbols == '=') base_count++;
					else
					{
						base++;
						break;
					}
					cached_last_terminal_symbol = previous_terminal_symbol;
				}
				if (base_count == 0)
				{
					base = last_terminal_position;
					while (!magazine_memory[base - 1].compare("E")) base--;
				}
				else while (!magazine_memory[base - 1].compare("E")) base--;

				while (magazine_current_size > base)
				{
					rule_string.insert(0, magazine_memory[magazine_current_size]);
					magazine_memory[magazine_current_size--] = "";
				}
				rule_string.insert(0, magazine_memory[magazine_current_size]);

				int rule_number = get_rule_number(rule_string);
				if (rule_number == 0)
				{
					reading_in_progress = false;
					break;
				}
				magazine_memory[magazine_current_size] = "E";
				rules_string.append(std::to_string(rule_number) + " ");
				break;
			}

			case ' ':
				reading_in_progress = false;
				break;

			default:
				reading_in_progress = false;
				break;
			}
		}
	}

public:

	parser(std::fstream* input_lexemes, std::fstream* output, std::fstream* tokens)
	{
		input_file = input_lexemes;
		input_file->seekg(0);
		output_file = output;
		tokens_file = tokens;
	}

	void do_parsing()
	{
		parse_input_chain();
		parse();
	}
};

