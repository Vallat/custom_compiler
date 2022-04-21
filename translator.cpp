#include "compiler.h"

class translator
{
private:
	struct node
	{
		std::string lexeme;
		std::vector<node*> childs;
	};

	std::fstream* tokens_file;
	std::fstream* rules_file;
	std::ofstream* tetrads_file;

	std::vector<std::string> list_of_tokens;
	std::vector<int> list_of_rules;

	std::string grammar_rules[RULES_AMOUNT] = { ";", "skip", "skip", "ifthenelse", "ifthen", "=", "or", "and", "not", "<", ">", "==", "+", "-", "/", "*", "=", "skip", "a", "c" };

	int number_of_last_step = 0;
	int jumps_counter = 0;

	void read_tokens()
	{
		std::string current_token;
		while (getline(*tokens_file, current_token))
		{
			list_of_tokens.push_back(current_token);
		}
	}

	void read_rules()
	{
		std::string current_rule;
		while (getline(*rules_file, current_rule, ' '))
		{
			list_of_rules.push_back(atoi(current_rule.c_str()));
		}
	}

	void put_tokens_back(node* root)
	{
		if (root == nullptr)
		{
			return;
		}

		if (root->lexeme == "a" || root->lexeme == "c")
		{
			root->lexeme = list_of_tokens.back();
			list_of_tokens.pop_back();
		}

		for (int i = (int)root->childs.size() - 1; i >= 0; i--)
		{
			put_tokens_back(root->childs[i]);
		}
	}

	node* create_new_node(std::string node_lexeme)
	{
		node* current_node = new node;
		current_node->lexeme = node_lexeme;
		return current_node;
	}

	void add_node(node* root, int rule_num)
	{
		std::string lexeme_from_rule = grammar_rules[rule_num-1];
		if (lexeme_from_rule.compare("skip"))	//not skip
		{
			root->lexeme = lexeme_from_rule;
			if (!lexeme_from_rule.compare("a") || !lexeme_from_rule.compare("c")) return;	//not "a" or not "c"

			if (!lexeme_from_rule.compare("="))
				root->childs.push_back(create_new_node("a"));
			else
				root->childs.push_back(create_new_node("E"));

			if (!lexeme_from_rule.compare("not")) return;

			root->childs.push_back(create_new_node("E"));
			if(!lexeme_from_rule.compare("ifthenelse"))
				root->childs.push_back(create_new_node("E"));
		}
	}

	void construct_tree(node* root)
	{
		int rule = list_of_rules.back();
		if (rule == 2 || rule == 3 || rule == 18)
		{
			add_node(root, rule);
			list_of_rules.pop_back();
			build_tree(root);
		}
		add_node(root, rule);
		if (!list_of_rules.empty())
			list_of_rules.pop_back();
		build_tree(root);
	}

	void build_tree(node* root)
	{
		for (int i = (int)root->childs.size() - 1; i >= 0; i--)
		{
			if (root->childs[i]->lexeme == "E")
			{
				int rule = list_of_rules.back();

				if (rule == 2 || rule == 3 || rule == 18)
				{
					add_node(root, rule);
					list_of_rules.pop_back();
					build_tree(root);
				}
				else
				{
					add_node(root->childs[i], rule);
					list_of_rules.pop_back();
					build_tree(root->childs[i]);
				}
			}
		}
	}

	void generate_tetrad(node* root)
	{
		if (root->lexeme == "ifthenelse")
		{
			//tree->isVisited = true;
			generate_tetrad(root->childs[0]);
			log_tetrad(root->lexeme, "", "", "");

			log_tetrad("JMP", "F" + std::to_string(jumps_counter), "", "");

			log_tetrad("FN", "", "", std::to_string(jumps_counter++));
			generate_tetrad(root->childs[2]);

			log_tetrad("FN", "", "", std::to_string(jumps_counter++));
			generate_tetrad(root->childs[1]);
		}
		else if (root->lexeme == "ifthen")
		{
			//tree->isVisited = true;
			generate_tetrad(root->childs[0]);
			log_tetrad(root->lexeme, "", "", "");

			log_tetrad("JMP", "F" + std::to_string(jumps_counter), "", "");

			log_tetrad("FN", "", "", std::to_string(jumps_counter++));
			generate_tetrad(root->childs[1]);
		}
		else if (root->lexeme == "=")
		{
			if (root->childs[1]->childs.empty())
			{
				log_tetrad(root->lexeme, root->childs[1]->lexeme, "", root->childs[0]->lexeme);
			}
			else
			{
				generate_tetrad(root->childs[1]);
				log_tetrad(root->lexeme, "R" + std::to_string(number_of_last_step), "", root->childs[0]->lexeme);
			}
		}
		else if (root->lexeme == "not")
		{
			generate_tetrad(root->childs[0]);
			log_tetrad(root->lexeme, "R" + std::to_string(number_of_last_step++), " ", "R" + std::to_string(number_of_last_step));
		}
		else
		{
			if (root->childs[0]->childs.empty())
			{
				if (root->childs[1]->childs.empty())
				{
					log_tetrad(root->lexeme, root->childs[0]->lexeme, root->childs[1]->lexeme, "R" + std::to_string(++number_of_last_step));
				}
				else
				{
					generate_tetrad(root->childs[1]);
					log_tetrad(root->lexeme, root->childs[0]->lexeme, "R" + std::to_string(number_of_last_step), "R" + std::to_string(++number_of_last_step));
				}
			}
			else
			{
				if (root->childs[1]->childs.empty())
				{
					generate_tetrad(root->childs[0]);
					log_tetrad(root->lexeme, "R" + std::to_string(number_of_last_step), root->childs[1]->lexeme, "R" + std::to_string(++number_of_last_step));
				}
				else
				{
					generate_tetrad(root->childs[0]);
					int cached_left_num = number_of_last_step;		//We cache it because it can be changed by prefix increment
					generate_tetrad(root->childs[1]);
					int cached_right_num = number_of_last_step;

					log_tetrad(root->lexeme, "R" + std::to_string(cached_left_num), "R" + std::to_string(cached_right_num), "R" + std::to_string(++number_of_last_step));
				}
			}
		}
	}

	void log_tetrad(const std::string& lexeme, const std::string& first_pointer, const std::string& second_pointer, const std::string& memory_register)
	{
		if (lexeme == "ifthenelse")
		{
			jumps_counter++;
			*tetrads_file << "\tif (R" + std::to_string(number_of_last_step) << ", " << second_pointer << ", F" + std::to_string(jumps_counter + 1) << ")" << std::endl;
		}
		else if (lexeme == "FN")
		{
			*tetrads_file << "F" << memory_register << ": \n";
		}
		else if(lexeme != ";")
		{
			*tetrads_file << "\t" << lexeme << " (" << first_pointer << ", " << second_pointer << ", " << memory_register << ")" << std::endl;
		}
		else
		{
			number_of_last_step--;
		}
	}

public:
	translator(std::ofstream* file_to_write, std::fstream* file_with_rules, std::fstream* file_with_tokens)
	{
		tetrads_file = file_to_write;
		rules_file = file_with_rules;
		rules_file->seekg(0);
		tokens_file = file_with_tokens;
		tokens_file->seekg(0);
	}

	void translate()
	{
		node* root = new node;

		read_rules();
		construct_tree(root);

		read_tokens();
		put_tokens_back(root);

		generate_tetrad(root);
	}
};
