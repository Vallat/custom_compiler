#include "compiler.h"

class c_lexeme_analyzer
{
private:
	enum class Analyzer_states
	{
		STATE_H,		//initial state
		STATE_T,		//then
		STATE_E,		//else
		STATE_A,		//and
		STATE_N,		//not
		STATE_I,		//if
		STATE_O, 		//or
		STATE_D,		//binary constants
		STATE_V,		//identificators
		STATE_C,		//comments
		STATE_ER,		//error
	};

	Analyzer_states current_state = Analyzer_states::STATE_H;
	int current_substate = 1;
	bool reading_in_progress = false;

	std::ifstream *reading_file;
	std::fstream* writing_file;
	char reading_char;
	std::string current_lexem;

	char lexeme_endings[3] = { ' ', '\n', '\t' };
	char compare_lexemes[3] = { '<', '>', '=' };
	char logical_lexemes[4] = { '+', '-', '*', '/' };
	char composite_lexemes[5] = { '{', '}', '(', ')', ';'};

	void write_to_log(std::string what_to_print)
	{
		*writing_file << what_to_print << "\n";
	}

	void set_state(Analyzer_states new_state)
	{
		current_state = new_state;
		current_substate = 1;
		current_lexem.clear();
	}

	void read_next_char()
	{
		if (!reading_file->get(reading_char))
			reading_in_progress = false;
	}

	void do_then_analyze()
	{
		read_next_char();
		if (current_substate == 4)
		{
			write_to_log(current_lexem.append(" - \"then\" Lexeme"));
			set_state(Analyzer_states::STATE_H);
		}
		else if (reading_char == "then"[current_substate])
		{
			current_substate++;
			current_lexem += reading_char;
			do_then_analyze();
		}
		else exit_from_key_words_handling();
	}
	
	void do_else_analyze()
	{
		read_next_char();
		if (current_substate == 4)
		{
			write_to_log(current_lexem.append(" - \"else\" Lexeme"));
			set_state(Analyzer_states::STATE_H);
		}
		else if (reading_char == "else"[current_substate])
		{
			current_substate++;
			current_lexem += reading_char;
			do_else_analyze();
		}
		else exit_from_key_words_handling();
	}

	void do_and_analyze()
	{
		read_next_char();
		if (current_substate == 3)
		{
			write_to_log(current_lexem.append(" - \"and\" Lexeme"));
			set_state(Analyzer_states::STATE_H);
		}
		else if (reading_char == "and"[current_substate])
		{
			current_substate++;
			current_lexem += reading_char;
			do_and_analyze();
		}
		else exit_from_key_words_handling();
	}

	void do_not_analyze()
	{
		read_next_char();
		if (current_substate == 3)
		{
			write_to_log(current_lexem.append(" - \"not\" Lexeme"));
			set_state(Analyzer_states::STATE_H);
		}
		else if (reading_char == "not"[current_substate])
		{
			current_substate++;
			current_lexem += reading_char;
			do_not_analyze();
		}
		else exit_from_key_words_handling();
	}

	void do_or_analyze()
	{
		read_next_char();
		if (reading_char == 'r')
		{
			current_lexem += reading_char;
			write_to_log(current_lexem.append(" - \"or\" Lexeme"));
			read_next_char();
			set_state(Analyzer_states::STATE_H);
		}
		else exit_from_key_words_handling();
	}

	void do_if_analyze()
	{
		read_next_char();
		if (reading_char == 'f')
		{
			current_lexem += reading_char;
			write_to_log(current_lexem.append(" - \"if\" Lexeme"));
			read_next_char();
			set_state(Analyzer_states::STATE_H);
		}
		else exit_from_key_words_handling();
	}

	void exit_from_key_words_handling()
	{
		std::string current_lexem_cache = current_lexem;
		if (!is_compare_lexeme() && !is_logical_lexeme() && !is_composite_lexeme())
		{
			set_state(Analyzer_states::STATE_V);
			current_lexem = current_lexem_cache;
			return;
		}
		if (!current_lexem_cache.empty())
			write_to_log(current_lexem_cache.append(" - Identificator Lexeme"));
		set_state(Analyzer_states::STATE_H);
	}

	bool do_special_lexemes_check()
	{
		if (is_compare_lexeme())
			found_compare_lexeme();
		else if (is_logical_lexeme())
			found_logical_lexeme();
		else if (is_composite_lexeme())
			found_composite_lexeme();
		else return false;
		set_state(Analyzer_states::STATE_H);
		return true;
	}

	bool is_compare_lexeme()
	{
		if (std::find(std::begin(compare_lexemes), std::end(compare_lexemes), reading_char) != std::end(compare_lexemes))
			return true;
		return false;
	}

	void found_compare_lexeme()
	{
		if (reading_char == '=')
		{
			read_next_char();
			if (!reading_in_progress)
			{
				set_state(Analyzer_states::STATE_ER);
				return;
			}
			set_state(Analyzer_states::STATE_H);
			if (reading_char == '=') {
				write_to_log("== - Compare Lexeme");
				read_next_char();
				return;
			}
			write_to_log("= - Assign Lexeme");
			return;
		}
		current_lexem += reading_char;
		write_to_log(current_lexem.append(" - Compare Lexeme"));
		read_next_char();
	}

	bool is_logical_lexeme()
	{
		if (std::find(std::begin(logical_lexemes), std::end(logical_lexemes), reading_char) != std::end(logical_lexemes))
			return true;
		return false;
	}

	void found_logical_lexeme()
	{
		if (reading_char == '/')
		{
			read_next_char();
			if (!reading_in_progress)
			{
				set_state(Analyzer_states::STATE_ER);
				return;
			}
			if (reading_char == '/') {
				do_comments_analyze();
				return;
			}
			write_to_log("/ - Logical Lexeme");
			set_state(Analyzer_states::STATE_H);
			return;
		}
		current_lexem += reading_char;
		write_to_log(current_lexem.append(" - Logical Lexeme"));
		read_next_char();
	}

	bool is_composite_lexeme()
	{
		if (std::find(std::begin(composite_lexemes), std::end(composite_lexemes), reading_char) != std::end(composite_lexemes))
			return true;
		return false;
	}

	void found_composite_lexeme()
	{
		current_lexem += reading_char;
		write_to_log(current_lexem.append(" - Composite Lexeme"));
		read_next_char();
	}

	bool is_ending_lexeme()
	{
		if (std::find(std::begin(lexeme_endings), std::end(lexeme_endings), reading_char) != std::end(lexeme_endings))
			return true;
		return false;
	}

	void do_binary_constants_analyze()
	{
		read_next_char();
		if (reading_char == '0' || reading_char == '1')
		{
			current_lexem += reading_char;
			do_binary_constants_analyze();
			return;
		}
		write_to_log(current_lexem.append(" - BinaryConstant Lexeme"));
		set_state(Analyzer_states::STATE_H);
	}

	void do_identificators_analyze()
	{
		read_next_char();
		if ((reading_char > 47 && reading_char < 58) || (reading_char > 64 && reading_char < 91) || (reading_char > 96 && reading_char < 123))
		{
			current_lexem += reading_char;
			do_identificators_analyze();
			return;
		}
		write_to_log(current_lexem.append(" - Identificator Lexeme"));
		set_state(Analyzer_states::STATE_H);
	}

	void do_comments_analyze()
	{
		read_next_char();
		if (reading_char != '\n')
		{
			current_lexem += reading_char;
			do_comments_analyze();
			return;
		}
		write_to_log(current_lexem.append(" - Comment"));
		set_state(Analyzer_states::STATE_H);
	}

public:
	c_lexeme_analyzer(std::ifstream* file_to_read, std::fstream* file_to_write)
	{
		reading_file = file_to_read;
		reading_file->seekg(0);
		writing_file = file_to_write;
		reading_char = NULL;
	}

	void do_lexeme_analyze()
	{
		int lines_counter = 1;
		reading_in_progress = true;
		read_next_char();
		while (reading_in_progress)
		{
			switch (current_state)
			{
			case c_lexeme_analyzer::Analyzer_states::STATE_H:
				switch (reading_char)
				{
				case 't':		//then lexeme
					set_state(Analyzer_states::STATE_T);
					break;
				case 'e':		//else lexeme
					set_state(Analyzer_states::STATE_E);
					break;
				case 'a':		//and lexeme
					set_state(Analyzer_states::STATE_A);
					break;
				case 'n':		//not lexeme
					set_state(Analyzer_states::STATE_N);
					break;
				case 'i':		//if lexeme
					set_state(Analyzer_states::STATE_I);
					break;
				case 'o':		//or lexeme
					set_state(Analyzer_states::STATE_O);
					break;
				case '1':		//binary constants
					set_state(Analyzer_states::STATE_D);
					break;
				case '/':		//check for comment
					set_state(Analyzer_states::STATE_C);
					break;
				default:
					if (is_ending_lexeme())		//skippable symbols
						read_next_char();
					else if ((reading_char > 64 && reading_char < 91) || (reading_char > 96 && reading_char < 123))
						set_state(Analyzer_states::STATE_V);		//identificators
					else if(!do_special_lexemes_check())
						set_state(Analyzer_states::STATE_ER);
					break;
				}
				break;
			case c_lexeme_analyzer::Analyzer_states::STATE_T:
				current_lexem += reading_char;
				do_then_analyze();
				break;
			case c_lexeme_analyzer::Analyzer_states::STATE_E:
				current_lexem += reading_char;
				do_else_analyze();
				break;
			case c_lexeme_analyzer::Analyzer_states::STATE_A:
				current_lexem += reading_char;
				do_and_analyze();
				break;
			case c_lexeme_analyzer::Analyzer_states::STATE_N:
				current_lexem += reading_char;
				do_not_analyze();
				break;
			case c_lexeme_analyzer::Analyzer_states::STATE_I:
				current_lexem += reading_char;
				do_if_analyze();
				break;
			case c_lexeme_analyzer::Analyzer_states::STATE_O:
				current_lexem += reading_char;
				do_or_analyze();
				break;
			case c_lexeme_analyzer::Analyzer_states::STATE_D:
				current_lexem += reading_char;
				do_binary_constants_analyze();
				break;
			case c_lexeme_analyzer::Analyzer_states::STATE_V:
				current_lexem += reading_char;
				do_identificators_analyze();
				break;
			case c_lexeme_analyzer::Analyzer_states::STATE_C:
				found_logical_lexeme();
				break;
			default:
				reading_in_progress = false;
				std::string error_handling_message = "Error occurred! The last symbol was ";
				write_to_log(error_handling_message + reading_char);
				break;
			}
		}
	}
};
