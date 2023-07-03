#include <iostream>
#include <string>
#include <ncurses.h>
#include <fstream>
#include <cmath>
#include <vector>
#include <cstring>
#include<unistd.h>
#include <trie.h>
//Things to settle:
//

#define normal_mode 0
#define insert_mode 1
#define command_mode 2

#define REG_COLOR_NUM 1
#define File_color 1
#define Information_color 3
#define Command_color 2
#define time_to_scroll 5

using namespace std;
using Lexicon = trie::trie_map<char, trie::SetCounter>;
using WordList = std::vector<std::string>;

int in;
int mode = normal_mode;
bool flag = 0;//flag indicates when I should move the cursor back to the File_window
bool file_existence = 1;
//used to show if the file user opened exists
bool is_read_only = 0;

struct Line{
    string oneline;
    int lines_indicator;
    int the_sequence_of_this_marked_line = 1;
};
//this array can be used to indicate the mark of lines
//   lines_indicator[actually_lines] = mark_lines;

int x = 0, y = 0;//used to get coordinate of the cursor
int last_x = 0, last_y = 0;//sometimes I have to save the coordinate of the cursor
int true_x = 0, true_y = 0;
int command_x = 1, command_y = 0;
int word_x = 0, word_y = 0;
//I finally find it convenient to have two coordinate variables to record the coordinate of each window
//not only the file_window
int the_true_length_of_the_true_x_position = 0;
//It helps to write the up and down keys
// bool cursor_is_on_the_left_side_of_true_position = 0;
bool has_up_down = 0;
vector<Line> Thelines;
vector<string> Marked_lines;
//I think I have to record the content of every line to accomplish some of the task
//then every time I change the content,I must renew the array
//I don't want to have the second vector at first, but later when I have to
//implement some keys, I think it would be much convienient if I have one
//vector to store all the marked_lines

int max_lines = 0;
vector<string> command;//It records the command the user type in
int newest_command = 0;
int now_command = 0;
//These two variables helps to wrire command history
string one_command_line;
bool has_saved = 0;
bool no_clear = 0;
fstream fs;
fstream fs_to_save;
fstream input_the_wordlist;
string firstword;
string secondword;
int zeropo, firstpo, secondpo, thirdpo;
int flag1, flag2, flag3, flag4, flag5;
//it helps to implement the substitution and debug
int have_how_much_scroll = 0;
int place;
vector<int> where_to_start_from_the_wordlist;
bool word_win_has_to_be_recreate = 0;
bool one_word_is_not_complete = 0;
string one_word;
WordList word;
Lexicon lexicon;
vector<string> the_whole_words;
int current_browse_word = 1;
//These are for the extension word completion
//some plan are deserted finally


WINDOW *File_window;
WINDOW *Information_window;
WINDOW *Command_window;
WINDOW *Word_window;


void Open_and_Present_the_FILE(char openmode[1000], char filename[1000]);
//I first use string as its parameter and some bugs occur
//So I have to use char[] to avoid some bugs
void Initialization();
void INITCOLOR();
// void REFRESH();
void insertsomething(int ch);
void Movethecursor(int ch);
//I have to give an int rather than a char cuz things like
//KEY_LEFT is bigger than 256
void Renew_the_cursor_info(char filename[1000]);
void get_present_find_the_command(char *filename);
void REPRINT();
vector<Line> renew_Thelines(vector<Line> Thelines);
void substitute();
void onesub(int index, int i);
void Word_completion();
//It's for the coolest extension:word completion!

int main(int argc, char *argv[])
// argc indicates the length of the argv array-of-strings
// argv is an array of char*,each of its member is a string from the command line
// argv[2] is exactly the filename of the file user want to open
// minivim [options] <filename>
{   
    Initialization();

    //FOR DEBUG:
    // for(int i = 0; i <= 25; i++)
    // {
    //     mvwprintw(File_window, i, 0,"%d", where_to_start_from_the_wordlist[i]);
    // }
    // wrefresh(File_window);
    // wprintw(File_window,"%s  %s",argv[1], argv[2]);

    //Then we need to open the file and present the contents on the filewindow
    Open_and_Present_the_FILE(argv[1], argv[2]);
    
    Renew_the_cursor_info(argv[2]);
    
    while(in = getch())
    {
        if(in == 'i' && mode == normal_mode && !is_read_only)
        //that means it's not read-only
        {
            mode = insert_mode;
            has_up_down = 0;
            getyx(File_window, y, x);
            last_x = x;
            last_y = y;
            Renew_the_cursor_info(argv[2]);
            wmove(File_window, last_y , last_x);//after the print,remember  to move the cursor back to the file window
            wrefresh(File_window);//At the first time , I write too much "wrefresh" and lead to some bug
            //so the key is ,write "wrefresh" only when it's necessary
            continue;
        }
        else if(in == ':' && mode == normal_mode)
        {
            mode = command_mode;
            getyx(File_window, y, x);
            last_x = x;
            last_y = y;
            flag = 1;//maybe it's redundant
            mvwprintw(Command_window, 0, 0,":");//print ":"
            wrefresh(Command_window);
            Renew_the_cursor_info(argv[2]);
            wmove(Command_window, 0 ,1);//we need to move the cursor here ,ready for user to type in the command
            wrefresh(Command_window);
            continue;
        }
        else if(in == 27)
        {
            if(mode != normal_mode)
            {
                getyx(File_window, y, x);
                Word_window = NULL;
                wrefresh(Word_window);
                REPRINT();
                wmove(File_window, y, x);
                wrefresh(File_window);
                mode = normal_mode;
                has_up_down = 0;
                if(flag == 1)
                {
                    one_command_line.clear();
                    wclear(Command_window);
                    wmove(File_window, last_y, last_x);//remember to move the cursor
                }
                Renew_the_cursor_info(argv[2]);
                wrefresh(Command_window);
            }   
        }
        if(mode == normal_mode)
        {
            getyx(File_window, y, x);
            Word_window = NULL;
            wrefresh(Word_window);
            REPRINT();
            wmove(File_window, y, x);
            wrefresh(File_window);
            Movethecursor(in);
            getyx(File_window, y, x);
            Renew_the_cursor_info(argv[2]);// in every turn I have to renew the cursor's line and column in the information window

        }
        else if(mode == insert_mode)
        {
            insertsomething(in);
            getyx(File_window, y, x);
            has_saved = 0;
            Renew_the_cursor_info(argv[2]);
        }
        else if(mode == command_mode)
        { 
            get_present_find_the_command(argv[2]);
            Renew_the_cursor_info(argv[2]);

            //FOR  DEBUG
            // getyx(Command_window, command_y, command_x);
            // wclear(File_window);
            // mvwprintw(File_window, 0, 0, "now_command == %d", now_command);
            // mvwprintw(File_window, 1, 0, ("one_command_line == " +  one_command_line).c_str());
            // if(command.size() > 0)
            // {
            //     mvwprintw(File_window, 2, 0, ("command[0]" + command[0]).c_str());
            // }
            // mvwprintw(File_window, 3, 0,  "%d %d %d %d %d %d %d %d %d", zeropo, firstpo, secondpo, thirdpo, flag1, flag2, flag3, flag4, flag5);
            // mvwprintw(File_window, 4, 0,  firstword.c_str());
            // mvwprintw(File_window, 5, 0, secondword.c_str());
            // mvwprintw(File_window, 5, 0, one_command_line.substr(0 ,3).c_str());
            // wrefresh(File_window);
            // wmove(Command_window, command_y, command_x);
            // wrefresh(Command_window);
            //FOR DEBUG
        }
        }
    fs.close();
    input_the_wordlist.close();
    endwin();//it's necessary to have it to deallocate the memory
    return 0;
}
void Open_and_Present_the_FILE(char openmode[1000], char filename[1000])
{
    if((string)openmode == "-r")
    {
        is_read_only = 1;
    }   //The funny thing is I don't write (string) at the first time and lead to bugs
    //the C-type string is so troublesome

    //FOR DEBUG
    // mvwprintw(Command_window, 0, 150,   openmode  );
    // wrefresh(Command_window);
    //

    // char filepath[1000] = "UserFile/";
    // strcat(filepath, filename);

    if((string)openmode == "-t")
    {
        fs.open((string)filename, ios::trunc);
        if(!fs)//That mens the file doesn't exist ,we have to create it
        {
            file_existence = 0;
        }
        Thelines.push_back({"", 1});
        Marked_lines.push_back("");
        //I forgot to write this in the first time and lead to segmentation fault
        fs.close();
        return;
    }
    else
    {
        fs.open((string)filename);
        if(!fs)//That mens the file doesn't exist ,we have to create it
        {
            file_existence = 0;
            Thelines.push_back({"", 1});
            Marked_lines.push_back("");
            return;
        }
        else
        {
            string line;
            while(getline(fs, line))
            {
                Marked_lines.push_back(line);
            }
            Thelines = renew_Thelines(Thelines);
            REPRINT();
            y = x = 0;
            wmove(File_window, y, x);
            fs.close();
        }
    }
}
    // int actually_lines = 1;
    // int mark_lines = 1;
    // the two pointer helps to tell us which lines were actually one line in
    //the file and were divided into several parts just cuz the
    //window is not big enough to contain the contents
    //Note that the mark of the lines start from 1
    //and vector sharts from 0

 
    // while(getline(fs, line))//It will end when it can't get any lines
    // {
    //     Marked_lines.push_back(line);
    //     if(line.size() <= COLS)
    //     {
    //         wprintw(File_window, line.c_str());
    //         //we have to change the string to c-string to pass as a parameter
    //         Thelines.push_back({line, mark_lines});
    //     }
    //     else
    //     {
    //         string temp = line.substr(COLS);
    //         while(temp.size())
    //         {
    //             Thelines.push_back({temp, mark_lines});
    //             wprintw(File_window, temp.c_str());
    //             temp = temp.substr(COLS);
    //         }
    //     }
    //     //the array record the length of every line in the file
    //     //thus making inserting possible
    //     mark_lines++;
    //     wrefresh(File_window);
    // }
void Initialization()
{
    initscr(); //start curses mode
    raw();//to disable line buffering
    cbreak();//line buffering disabled,pass on everything to me
    //however,I don't know the difference between raw and cbreak
    noecho();// perhaps this function does not matter?
    keypad(stdscr, true);

    // check window size is illegal
    // LINES and COLS store the total line and cols of stdscr
    if (LINES < 5)
    {
        fprintf(stderr, "window line size is small than 5");
        endwin();
        exit(-1);
    }//from the demo

     //First we need to create 3 windows
    File_window = newwin(LINES - 2, COLS, 0, 0);
    wrefresh(File_window);
    Information_window = newwin(1, COLS, LINES - 2, 0);
    wrefresh(Information_window);
    Command_window = newwin(1, COLS, LINES - 1, 0);
    wrefresh(Command_window);
    
    keypad(File_window, TRUE);// some special keys are allowed
    keypad(Information_window, TRUE);
    keypad(Command_window, TRUE);

    // wborder(File_window,'~',' ',' ',' ','~',' ','~',' ');
    // wrefresh(File_window);
    //I tried to print'~' but it didn't work as I wished

    //This is  for the extension 1：word completion !!!
    //First ,I have to open the file
    char file_of_word[30];
    strcpy(file_of_word, "words_alpha.txt");
    //DEBUG:
    // wclear(Command_window);
    // mvwprintw(Command_window, 0, 0, "%s ", file_of_word);
    // wrefresh(Command_window);

    input_the_wordlist.open((string)file_of_word);
    int i = 0;//At this time 0 is for a and 1 is for b so 25 is for z
    string line;
    while(getline(input_the_wordlist, line))
    {
        word.push_back(line);
    }
    for (const auto &word1 : word) 
    {   
       lexicon.insert(word1);
   }
    //This is to initialize the lexicon
    // int row = 0;
    //I come up with a BUG here,and it takes me so long to figure it out
    //I can use the raw words_alpha.txt for some unknown reasons
    //I can only create one by myself or I will get wrong

    // //FOR DEBUG
    // getline(input_the_wordlist, line);
    // wclear(Command_window);
    // mvwprintw(Command_window, 0, 3, "%s %d", line.c_str(), line.size());
    // wrefresh(Command_window);
    // where_to_start_from_the_wordlist.push_back(0);
    // where_to_start_from_the_wordlist.resize(30);

    // while(i <= 25 && getline(input_the_wordlist, line))
    // {
    //     row++; 
    //     if(line[0] > i + (int)'a')
    //     {
    //         i++;
    //         where_to_start_from_the_wordlist[i] = row; 
    //     }
    //     line.clear();
    // }


    INITCOLOR();
}
void INITCOLOR()
{
    if(has_colors() == FALSE)
    {
        endwin();
        fs.close();
        input_the_wordlist.close();
        exit(-1);
    }
    //That means your terminal does not support color

    // start_color();
    // init_pair(REG_COLOR_NUM, COLOR_WHITE, COLOR_BLACK);
    // init_pair(CUS_COLOR_NUM, COLOR_YELLOW, COLOR_CYAN);

    // wbkgd(stdscr, COLOR_PAIR(REG_COLOR_NUM));
    // wrefresh(stdscr);

    start_color();
    // init color pair
    init_pair(REG_COLOR_NUM, COLOR_WHITE, COLOR_BLACK); 
    init_pair(File_color, COLOR_WHITE, COLOR_BLACK);
    init_pair(Information_color, COLOR_WHITE, COLOR_CYAN);
    // set window color
    wbkgd(stdscr, COLOR_PAIR(REG_COLOR_NUM));
    wrefresh(stdscr);//I'm not sure if this one can be deleted

    wbkgd(File_window, COLOR_PAIR(File_color));
    wrefresh(File_window);

    wbkgd(Information_window,COLOR_PAIR(Information_color));
    wrefresh(Information_window);

    wbkgd(Command_window, COLOR_PAIR(File_color));
    wrefresh(Command_window);
}

// void REFRESH()
// {
//     wrefresh(File_window);
//     wrefresh(Information_window);
//     wrefresh(Command_window);
// }
void insertsomething(int ch)
{
    if(ch >= '1' && ch <= '5' && Word_window != NULL)
    {
        //That means the user can now choose the word to complete
        string theword = the_whole_words[ch - '1' +  (current_browse_word - 1)];
        getyx(File_window, y, x);
        current_browse_word = 1;
        one_word_is_not_complete = 0;
        the_whole_words.clear();
        Word_window = NULL;
        Marked_lines[y].insert(x, theword.substr(one_word.size()));
        Thelines = renew_Thelines(Thelines);
        REPRINT();
        wrefresh(File_window);
        x += theword.substr(one_word.size()).size();
        wmove(File_window, y, x);
        wrefresh(File_window);
        return;
    }

    if(ch == KEY_LEFT && Word_window != NULL)
    {
        //That means the user can browse the word window,now
        if(current_browse_word == 1)
        {
            return;
        }
        else
        {
            current_browse_word--;
            Word_completion();
            // word_x -= 15;
            // wmove(Word_window, 0, word_x);
            // wrefresh(Word_window);
            return;
        }
    }

    if(ch == KEY_RIGHT && Word_window != NULL)
    {
        if(current_browse_word <= the_whole_words.size() - 1)
        {
            current_browse_word++;
            Word_completion();
            // word_x += 15;
            // wmove(Word_window, 0, word_x);
            // wrefresh(Word_window);    
            return;
        }
    }
    if(!isalpha(ch) && ch != KEY_BACKSPACE && ch != KEY_ENTER && ch != 10)//If I get a character that is not a alpha then I shouldn't display the 
    //shu ru fa window, I think (But display a blank window seems also fine)
    {
        //When key is backspace or is enter ,I have speciall part of code to 
        //control the result for the word completion
        one_word_is_not_complete = 0;
        one_word.clear();
        getyx(File_window, y, x);
        Word_window = NULL;//I try some way to hide the word window, and find let the window pointer
        //be NULL is the most convenient one
        wrefresh(Word_window);
        wmove(File_window, y, x);
        wrefresh(File_window);
    }
    if(isalpha(ch) && one_word_is_not_complete)
    {
        one_word.push_back(ch);
    }
    if(isalpha(ch) && !one_word_is_not_complete)
    {
        one_word_is_not_complete = 1;
        one_word.push_back(ch);
    }
    if(ch !=KEY_UP && ch !=KEY_DOWN)
    {
        has_up_down = 0;
    }
    
    if(ch == KEY_ENTER || ch == 10)//when I press enter ,I have tested the corresponding char is 10
    {
        if(Word_window != NULL)
        {
            //That means the word_window is displaying now
            Word_window = NULL;
            getyx(File_window, y, x);
            wrefresh(Word_window);
            REPRINT();
            wmove(File_window, y, x);
            wrefresh(File_window);
            return;
        }
        getyx(File_window, y, x);
        //for the scroll
        place = y + have_how_much_scroll + 1;//I have to record the y place to get back before I refresh the window
        y += have_how_much_scroll;
        if(y == Thelines.size() - 1 && x == Thelines[y].oneline.size() - 1)
        {
            Marked_lines.push_back("");
            Thelines.push_back({"", Thelines[y - 1].lines_indicator + 1, 1});
        }
        else if(x == 0)
        {
            Marked_lines.insert(Marked_lines.begin() + Thelines[y].lines_indicator - 1,"");
            Thelines = renew_Thelines(Thelines);
        }
        else
        {
            Marked_lines.insert(Marked_lines.begin() + Thelines[y].lines_indicator, Marked_lines[Thelines[y].lines_indicator - 1].substr(x + COLS * (Thelines[y].the_sequence_of_this_marked_line - 1)));
        
        //FOR DEBUG:
        // Marked_lines.insert(Marked_lines.begin() , "123");
        // Marked_lines.push_back("123");
        
            Marked_lines[Thelines[y].lines_indicator - 1] = Marked_lines[Thelines[y].lines_indicator - 1].substr(0, (x + COLS * (Thelines[y].the_sequence_of_this_marked_line - 1)));
            Thelines = renew_Thelines(Thelines);
        }
        y -= have_how_much_scroll;
        if(Thelines.size() == LINES - 2 && y == LINES - 3)
        {
            have_how_much_scroll ++;
        }
        //I have to scroll one line down
        else if(Thelines.size() > LINES - 2 && y > LINES - time_to_scroll)
        {
            have_how_much_scroll++;
        }
        getyx(File_window, y, x);
        REPRINT();
        x = 0;
        y = place;
        wmove(File_window, y, x);
        wrefresh(File_window);
    }

    else if(ch == KEY_LEFT)
    {
        getyx(File_window, y, x);
        y += have_how_much_scroll;
        if(x == 0 && y > 0 && Thelines[y - 1].lines_indicator == Thelines[y].lines_indicator && !have_how_much_scroll)
        {
            y--;
            x = COLS - 1;
        }
        //!!!something wrong with these codes above cuz 
        else if(x > 0)
        {
            x--;   
        }
        y -= have_how_much_scroll;
        wmove(File_window, y ,x);
        wrefresh(File_window);
        return;
    }

    else if(ch == KEY_RIGHT)
    {
        getyx(File_window, y, x);
        y += have_how_much_scroll;
        if(x < Thelines[y].oneline.size() && x < COLS)
        //it means x at most get to the last character  + 1 of this line
        //note that x starts from 0 
        {
            x++;
        }
        else if(x == COLS - 1 && y < Thelines.size() - 1 && Thelines[y + 1].lines_indicator == Thelines[y].lines_indicator && !have_how_much_scroll)
        {
            y++;
            x = 0;
        }
        y -= have_how_much_scroll;
        wmove(File_window, y, x);
        wrefresh(File_window);
        return;
    }
    else if(ch == KEY_UP)
    {
        if(!has_up_down)//That means there's no successive up and down
        {
            has_up_down = 1;
            getyx(File_window, y, x);
            true_x = x;
            true_y = y;
            the_true_length_of_the_true_x_position = COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + true_x;
        }
//to control the scroll
        last_y = y;
        last_x = x;
        if(y == 0 && have_how_much_scroll > 0)
        {
            have_how_much_scroll --;
        }
        REPRINT();
        y = last_y;
        x = last_x;
        wmove(File_window, y, x);
        if(Thelines[y].lines_indicator - 1 > 0)//That means when it hit the highest row then I don't need to do anything
        {
            if(the_true_length_of_the_true_x_position  + 1 <= Marked_lines[Thelines[y].lines_indicator - 2].size())
            {   
                int move_how_much_line = ceil(Marked_lines[Thelines[y].lines_indicator - 2].size() * 1.0 / COLS);
                //just move the length of lines of this marked line
                wmove(File_window, y - move_how_much_line, true_x);
            }
            else
            {
                int i;
                for(i = y; i > 0 && Thelines[i].lines_indicator - 1 == Thelines[y].lines_indicator - 1; i--);
                //To find the last row of the last marked line
                if(Thelines[i].oneline.size() > 0)
                {
                    wmove(File_window, i, Thelines[i].oneline.size() - 1);
                }
                else
                {
                    wmove(File_window, i, 0);
                }
                //I have to take special attention to the zero line situation or I will get some bugs
            }
        }
        wrefresh(File_window);
    return;
    }    
    
    else if(ch == KEY_DOWN)
    {
        if(!has_up_down)//That means there's no successive up and down
    {
        has_up_down = 1;
        getyx(File_window, y, x);
        true_x = x;
        true_y = y;//I must store the true position of y,x cuz the position of
        //we need it to implement future move
        the_true_length_of_the_true_x_position = COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + true_x;
    }
    //to control the scroll ,note that we can not reach beyond the lowest bound
        if(y >= LINES - 3 && Thelines.size() > LINES - 2 && Thelines.size() > y + have_how_much_scroll  + 1)
        {
            have_how_much_scroll++;
        }
        last_y = y;
        last_x = x;
        REPRINT();
        y = last_y;
        x = last_x;
        wmove(File_window, y, x);
    if(Thelines[y].lines_indicator - 1 < Marked_lines.size() - 1)//That means when it hit the lowest row and I don't need to do anything
    {
        if(the_true_length_of_the_true_x_position + 1 <= Marked_lines[Thelines[y].lines_indicator].size())
        {   //I have to use true_y instead of y!! cuz y can be changed all the time!
        //but the rule is true_y never changes if you press successive up and down
            int move_how_much_line;
            if(Marked_lines[Thelines[y].lines_indicator - 1].size())
            {
               move_how_much_line = ceil(Marked_lines[Thelines[y].lines_indicator - 1].size() * 1.0 / COLS);
            }
            else
            {
                move_how_much_line = 1;//If this line is an empty line then I have to move 1 row
            }
            wmove(File_window, y + move_how_much_line, true_x);
        }
        else
        {
            int i;
            for(i = y; i < Thelines.size() && Thelines[i].lines_indicator <= Thelines[y].lines_indicator + 1; i++);
            //To find the last row of the next marked line
            if(Thelines[i - 1].oneline.size() > 0)
            {
                wmove(File_window, i - 1, Thelines[i - 1].oneline.size() - 1);
            }
            else
            {
                wmove(File_window, i - 1, 0);
            }
        }
    }
    wrefresh(File_window);
    return;
    }
    else if(ch == KEY_BACKSPACE)
    {
        getyx(File_window, y, x);
        if(y == 0 && Thelines.size() >= LINES - 1 && have_how_much_scroll > 0)
        {
            have_how_much_scroll --;
        }
        //I have to scroll upside
        if(x > 0)
        { 
            x--;  
            wmove(File_window, y ,x);
            //It means the user delete one word and we should do as the shu ru fa do
            if(isalpha(Thelines[y].oneline[x]) && x > 0 && isalpha(Thelines[y].oneline[x - 1]) && one_word_is_not_complete)
            {
                one_word.pop_back();  
                Word_completion();
            }
            else
            {
                one_word_is_not_complete = 0;
                one_word.clear();
                getyx(File_window, y, x);
                Word_window = NULL;
                wrefresh(Word_window);
                REPRINT();
                wmove(File_window, y, x);
                wrefresh(File_window);
            }
            mvwdelch(File_window, y, x);
            Thelines[y].oneline.erase(x, 1);//I have to erase this character in the string
            Marked_lines[Thelines[y].lines_indicator - 1].erase(COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x, 1);
            wmove(File_window, y ,x);
            wrefresh(File_window);  
        }
        else if(x == 0 && y > 0 && Thelines[y].lines_indicator == Thelines[y - 1].lines_indicator)
        {
            y--;
            x = COLS - 1;
            mvwdelch(File_window, y, x);
            Thelines[y].oneline.erase(x, 1);//I have to erase this character in the string
            Marked_lines[Thelines[y].lines_indicator - 1].erase(COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x, 1);
            wmove(File_window, y ,x);
            wrefresh(File_window);
        }
        else if(x == 0 && y > 0 && Thelines[y].lines_indicator != Thelines[y - 1].lines_indicator)
        {
            last_y = y - 1;
            if(Thelines[y - 1].oneline.size())
            {
                last_x = Thelines[y - 1].oneline.size() - 1;
            }
            else if(!Thelines[y - 1].oneline.size())
            {
                last_x = 0;
            }
            Marked_lines[Thelines[y - 1].lines_indicator - 1] += Marked_lines[Thelines[y].lines_indicator - 1];
            Marked_lines.erase(Marked_lines.begin() + Thelines[y].lines_indicator - 1);
            //It's very easy to forget "- 1" and I write so many bugs at the first time
            Thelines = renew_Thelines(Thelines);
            REPRINT();
            x = last_x;
            y = last_y;
            wmove(File_window, y, x);
        }
    }
    
    // else if(ch == KEY_BACKSPACE)
    // {
    //     getyx(File_window, y, x);
    //     Thelines[y].oneline.erase(x, 1);
    //     //use erase to simplify
    //     if(x == 0 && y > 0) //this means I have to change the lines totaly
    //     //I must follow the rules of vim
    //     {
    //        mvwprintw(File_window, y - 1, Thelines[y - 1].oneline.size(), Thelines[y].oneline.c_str());
    //        //move the cursor to the coordinate to let y join 
    //        Thelines[y - 1].oneline += Thelines[y].oneline;
    //        Thelines.erase(Thelines.begin() + y);
    //        Marked_lines.erase(Marked_lines.begin() + COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x + 1);
    //        int i = y;
    //        while(Thelines[i].lines_indicator == Thelines[y].lines_indicator)
    //        {
    //           i++;
    //        }
    //        for(; i < Thelines.size(); i++)
    //        {
    //         Thelines[i].lines_indicator --;
    //        }
    //        wdeleteln(File_window);
    //        wmove(File_window, y - 1, Thelines[y - 1].oneline.size());
    //        wrefresh(File_window);
    //     }
    //     else if(x > 0)//just delete the character
    //     {
    //         x--;
    //         mvwdelch(File_window, y, x);
    //         wmove(File_window, y ,x);
    //     }
    //     wrefresh(File_window);
    // }
    //above was my first-time idea to implement backspace,BUT it's wrong in some situations

    else if(ch == 9)//that means the user type in 'TAB'
    {
        y += have_how_much_scroll;
        for(int i = 0; i < 4; i++)
        {   
            Marked_lines[Thelines[y].lines_indicator - 1].insert(COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x, 1, ' ');
            if(x < COLS - 1)
            {
                //Thelines[y].oneline.insert(x, 1, ' ');
                //Marked_lines[Thelines[y].lines_indicator - 1].insert(COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x, 1, ' ');
                //this row of code I renew the true-line(by line mark)
                x++;//this means I don't have to change the line
            }    
            else
            {
                y++;
                x = 0;
                //Thelines.insert(Thelines.begin() + y, {" ", Thelines[y - 1].lines_indicator});
                //Thelines[y].the_sequence_of_this_marked_line += Thelines[y - 1].the_sequence_of_this_marked_line; 
                //Marked_lines[Thelines[y].lines_indicator - 1].insert(COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x, 1, ' ');   
                //this means I have to create a new line:why not use insert function
                //I have to let sequence+1 to facilitate the print of the column mark
            }
            Thelines = renew_Thelines(Thelines);
            REPRINT();
            y -= have_how_much_scroll;
            wmove(File_window, y, x);
            wrefresh(File_window);
        }
        wrefresh(File_window);
        return;
    }

    else if(ch == KEY_DC)
    {
        getyx(File_window, y, x);
        y += have_how_much_scroll;
        mvwdelch(File_window, y, x);
        Thelines[y].oneline.erase(x, 1);//I have to erase this character in the string
        Marked_lines[Thelines[y].lines_indicator - 1].erase(COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x, 1);
        wrefresh(File_window);
    }

    else
    { 
        getyx(File_window, y, x);
        y += have_how_much_scroll;
        // winsch(File_window, ch);
        // if(x < COLS - 1)
        // {
        //     Thelines[y].oneline.insert(x, 1, ch);
        //     x++;
        // }    
        // else
        // {
        //     y++;
        //     x = 0;
        //     Thelines.insert(Thelines.begin() + y , {"", Thelines[y - 1].lines_indicator});
        //     Thelines[y].the_sequence_of_this_marked_line += Thelines[y - 1].the_sequence_of_this_marked_line;               
        // }
        //Things above is my first idea to implement the insert ,BUT is=t leaves me so much trouble so I finally desert it

        Marked_lines[Thelines[y].lines_indicator - 1].insert(COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x, 1, ch);
        Thelines = renew_Thelines(Thelines);
        REPRINT();
        if(x < COLS - 1)
        {
            x++;
        }    
        else
        {
            y++;
            x = 0;  
            Thelines.insert(Thelines.begin() + y , {"", Thelines[y - 1].lines_indicator});
            //If I do not do this to ask for a new place for Thelines,it will lead to segmentation fault
            //Thelines[y].the_sequence_of_this_marked_line += Thelines[y - 1].the_sequence_of_this_marked_line;        
        }
    }
        y -= have_how_much_scroll;
        wmove(File_window, y, x);
        wrefresh(File_window);

        //This is the extension 1: word completion!!!
        if(isalpha(ch))//Only when the user type in alpha charater should we execute the word completion part
        {
            Word_completion();
        }
        wmove(File_window, y, x);
        getyx(File_window, y, x);
        wrefresh(File_window);
}

void Movethecursor(int ch)
{
    if(ch != 'j' && ch != 'k' && ch !=KEY_UP && ch !=KEY_DOWN)
    {
        has_up_down = 0;
    }
    //This helps to implement up and down
    switch (ch)
    {
    case 'h':case KEY_LEFT:
     getyx(File_window, y, x);
     y += have_how_much_scroll;
    if(x == 0 && y > 0 && Thelines[y - 1].lines_indicator == Thelines[y].lines_indicator && !have_how_much_scroll)
    {
        y--;
        x = COLS - 1;
    }
    //!!!something wrong with these codes above cuz 
    else if(x > 0)
    {
        x--;   
    }
    y -= have_how_much_scroll;
    wmove(File_window, y ,x);
    wrefresh(File_window);
    break;

     case 'l':case KEY_RIGHT:
    getyx(File_window, y, x);
    y += have_how_much_scroll;
    if(x < Thelines[y].oneline.size() - 1 && x < COLS)
    //it means x at most get to the last character  + 1 of this line
    //note that x starts from 0 
    {
        x++;
    }
    else if(x == COLS - 1 && y < Thelines.size() - 1 && Thelines[y + 1].lines_indicator == Thelines[y].lines_indicator && !have_how_much_scroll)
    {
        y++;
        x = 0;
    }
    y -= have_how_much_scroll;
    wmove(File_window, y, x);
    wrefresh(File_window);
    break;

    case KEY_BACKSPACE:
    getyx(File_window, y, x);
    if(x > 0)
    {
        x--;
        wmove(File_window, y ,x);
        wrefresh(File_window);  
    }
    if(y == 0 && Thelines.size() >= LINES - 1 && have_how_much_scroll > 0)
    {
        have_how_much_scroll --;
    }
    else if(x == 0 && y > 0)
    {
        y--;
        if(Thelines[y].oneline.size() == 0)
        {
            x = 0;
        }
        else
        {
            x = Thelines[y].oneline.size() - 1;
        }
        wmove(File_window, y, x);
        wrefresh(File_window);
    } //That means back to the last charater in the last size
    break;

    case KEY_DC://when the user press delete
    getyx(File_window, y, x);
    y += have_how_much_scroll;
    mvwdelch(File_window, y, x);
    Marked_lines[Thelines[y].lines_indicator - 1].erase(COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x, 1);
    y -= have_how_much_scroll;
    wrefresh(File_window);
    break;

    case 'j': case KEY_UP:
    if(!has_up_down)//That means there's no successive up and down
    {
        has_up_down = 1;
        getyx(File_window, y, x);
        true_x = x;
        true_y = y;
        the_true_length_of_the_true_x_position = COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + true_x;
    }
    if(Thelines[y].lines_indicator - 1 > 0)//That means when it hit the highest row then I don't need to do anything
    {
        if(the_true_length_of_the_true_x_position  + 1 <= Marked_lines[Thelines[y].lines_indicator - 2].size())
        {   
            int move_how_much_line = ceil(Marked_lines[Thelines[y].lines_indicator - 2].size() * 1.0 / COLS);
            //just move the length of lines of this marked line
            wmove(File_window, y - move_how_much_line, true_x);
        }
        else
        {
            int i;
            for(i = y; i > 0 && Thelines[i].lines_indicator - 1 == Thelines[y].lines_indicator - 1; i--);
            //To find the last row of the last marked line
            if(Thelines[i].oneline.size() > 0)
            {
                wmove(File_window, i, Thelines[i].oneline.size() - 1);
            }
            else
            {
                wmove(File_window, i, 0);
            }
            //I have to take special attention to the zero line situation or I will get some bugs
        }
    }
    wrefresh(File_window);
    break;     
    
    case 'k': case KEY_DOWN:
    if(!has_up_down)//That means there's no successive up and down
    {
        has_up_down = 1;
        getyx(File_window, y, x);
        true_x = x;
        true_y = y;//I must store the true position of y,x cuz the position of
        //we need it to implement future move
        the_true_length_of_the_true_x_position = COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + true_x;
    }
    if(Thelines[y].lines_indicator - 1 < Marked_lines.size() - 1)//That means when it hit the lowest row and I don't need to do anything
    {
        if(the_true_length_of_the_true_x_position + 1 <= Marked_lines[Thelines[y].lines_indicator].size())
        {   //I have to use true_y instead of y!! cuz y can be changed all the time!
        //but the rule is true_y never changes if you press successive up and down
            int move_how_much_line;
            if(Marked_lines[Thelines[y].lines_indicator - 1].size())
            {
               move_how_much_line = ceil(Marked_lines[Thelines[y].lines_indicator - 1].size() * 1.0 / COLS);
            }
            else
            {
                move_how_much_line = 1;//If this line is an empty line then I have to move 1 row
            }
            //just move the length of lines of this marked line
            wmove(File_window, y + move_how_much_line, true_x);
        }
        else
        {
            int i;
            for(i = y; i < Thelines.size() && Thelines[i].lines_indicator <= Thelines[y].lines_indicator + 1; i++);
            //To find the last row of the next marked line
            if(Thelines[i - 1].oneline.size() > 0)
            {
                wmove(File_window, i - 1, Thelines[i - 1].oneline.size() - 1);
            }
            else
            {
                wmove(File_window, i - 1, 0);
            }
        }
    }
    wrefresh(File_window);
    break;  
    // if(y < Thelines.size() - 2)
    // {
    //     int i;
    //     int end_of_this_marked_line;i
    //     for(i = y + 1; i < Thelines.size(); i++)
    //     {
    //         if(Thelines[y].lines_indicator == Thelines[i].lines_indicator)
    //         {
    //             i++;
    //         }
    //     }
    //     if(i < Thelines.size())
    //     {
    //         end_of_this_marked_line = i;
    //         while(end_of_this_marked_line < Thelines.size() 
    //         && Thelines[i].lines_indicator == Thelines[end_of_this_marked_line].lines_indicator)
    //         {
    //             end_of_this_marked_line ++;
    //         }//I want to find the end of this line(marked line)
    //         end_of_this_marked_line --;//the while loop do one extra ++

    //         int len_of_this_line = COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) 
    //           + Thelines[end_of_this_marked_line].oneline.size();

    //         if(COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x
    //             <= len_of_this_line )//That means we can find place to move,we don't need to get leftward
    //         {
    //             wmove(File_window, i + Thelines[y].the_sequence_of_this_marked_line - 1, x);
    //         }
    //         else
    //         {
    //             wmove(File_window, end_of_this_marked_line, Thelines[end_of_this_marked_line].oneline.size() - 1);
    //         }
    //     }

    //above was my first-time idea,but it's complicating the simple thing ,so I use a more simpler way 

        case 10: case KEY_ENTER:
        {
        getyx(File_window, y, x);
        //for the scroll
        place = y + have_how_much_scroll + 1;//I have to record the y place to get back before I refresh the window
        y += have_how_much_scroll;
        if(y == Thelines.size() - 1 && x == Thelines[y].oneline.size() - 1)
        {
            Marked_lines.push_back("");
            Thelines.push_back({"", Thelines[y - 1].lines_indicator + 1, 1});
        }
        else if(x == 0)
        {
            Marked_lines.insert(Marked_lines.begin() + Thelines[y].lines_indicator - 1,"");
            Thelines = renew_Thelines(Thelines);
        }
        else
        {
            Marked_lines.insert(Marked_lines.begin() + Thelines[y].lines_indicator, Marked_lines[Thelines[y].lines_indicator - 1].substr(x + COLS * (Thelines[y].the_sequence_of_this_marked_line - 1)));
        
        //FOR DEBUG:
        // Marked_lines.insert(Marked_lines.begin() , "123");
        // Marked_lines.push_back("123");
        
            Marked_lines[Thelines[y].lines_indicator - 1] = Marked_lines[Thelines[y].lines_indicator - 1].substr(0, (x + COLS * (Thelines[y].the_sequence_of_this_marked_line - 1)));
            Thelines = renew_Thelines(Thelines);
        }
        y -= have_how_much_scroll;
        if(Thelines.size() == LINES - 2 && y == LINES - 3)
        {
            have_how_much_scroll ++;
        }
        //I have to scroll one line down
        else if(Thelines.size() > LINES - 2 && y > LINES - time_to_scroll)
        {
            have_how_much_scroll++;
        }
        getyx(File_window, y, x);
        REPRINT();
        x = 0;
        y = place;
        wmove(File_window, y, x);
        wrefresh(File_window);
        break;
        }
    }
}
void Renew_the_cursor_info(char filename[1000])
{
    if(mode == command_mode)
    {
        getyx(Command_window, command_y, command_x);
    }
    else
    {
        getyx(File_window, y, x);
        y += have_how_much_scroll;
    }
    switch(mode)
    {
        case normal_mode:
        mvwprintw(Information_window, 0, 0, "--Normal--   ");
        break;
        case insert_mode:
        mvwprintw(Information_window, 0, 0, "--Insert--   ");
        break;
        case command_mode:
        mvwprintw(Information_window, 0, 0, "--Command--  ");
    }//Just keep the information
    mvwprintw(Information_window, 0, 60, "Cursor line: %d    ,Cursor column: %d   ", Thelines[y].lines_indicator,COLS * (Thelines[y].the_sequence_of_this_marked_line - 1) + x + 1);
    mvwprintw(Information_window, 0, 30, "--Filename: %s--    ", filename);

    // // //FOR DEBUG
    // mvwprintw(Information_window, 0, 100, "Thelines %d Marked %d   ",Thelines.size(), Marked_lines.size());
    // mvwprintw(Information_window, 0, 15, "Mark[0]size%d   ",Marked_lines[0].size());
    // mvwprintw(Command_window, 0, 0, "y %d x %d capture: %d Thelines[0] %d indicator[0] %d Sequence[0] %d  position %d  Mark[0]size %d  ", y, x ,in, Thelines[0].oneline.size(), Thelines[0].lines_indicator, Thelines[0].the_sequence_of_this_marked_line, the_true_length_of_the_true_x_position, Marked_lines[0].size());
    // if(Thelines.size() > 1)
    // {
    //     mvwprintw(Command_window, 0 , 90, "Thelines[1] %d indicator[1] %d Sequence[1] %d place: %d  scroll:%d", Thelines[1].oneline.size(), Thelines[1].lines_indicator, Thelines[1].the_sequence_of_this_marked_line, place, have_how_much_scroll);
    // }
    // wrefresh(Command_window);
    // if(the_5_words.size() > 2)
    // {
    //     mvwprintw(Command_window, 0, 0, ((the_5_words[0]) + "  " + the_5_words[1]).c_str());
    //     wrefresh(Command_window);
    // }
    // // //FOR DEBUG

    if(mode == command_mode)
    {
        wmove(Command_window, command_y, command_x);
        //After I do this, there won't be an extra annoying cursor in the infomation window
        //I don't know why but I can't just use argv[2] for %s
        wrefresh(Information_window); 
        wrefresh(Command_window);
    }
    // else if(mode == insert_mode && Word_window != NULL &&
    // the_whole_words.size() && in != 9 && in != 32 && isalpha(in))//32 is for the blank space
    // {
    //     wmove(Word_window, 0, word_x);
    //     wrefresh(Word_window);
    // }
    // else if(mode == insert_mode && Word_window != NULL && (in == KEY_LEFT || in == KEY_RIGHT))
    // {
    //     wmove(Word_window, 0, word_x);
    //     wrefresh(Word_window);
    // }
    else
    {
        y -= have_how_much_scroll;
        wmove(File_window, y, x);
        //After I do this, there won't be an extra annoying cursor in the infomation window
        //I don't know why but I can't just use argv[2] for %s
        wrefresh(Information_window); 
        wrefresh(File_window);
    }
}

void get_present_find_the_command(char *filename)
{   
    if(in == KEY_ENTER || in == 10)//10 is for enter on my computer
    {
        command.push_back(one_command_line);
        wclear(Command_window);
        if(one_command_line == "w")
        {
            // char filepath[1000] = "UserFile/";
            // strcat(filepath, filename);
            ofstream outing((string)filename,ios::out|ios::trunc);
            //it's used for deleting the file and pour all the things in the window to it
            for(int i = 0; i < Marked_lines.size(); i++)
            {
                outing << Marked_lines[i] << endl;
            } 
            has_saved = 1;
            outing.close();
        }
        else if(one_command_line == "q")
        {
            if(has_saved)
            {
                fs.close();
                exit(0);//to terminate the program if saved
            }
            else
            {
                mvwprintw(Command_window, 0, 0, "The file is changed without saving.Using q! to enforce the exit.");
                wrefresh(Command_window);
                no_clear =  1;       
                sleep(3);
                wclear(Command_window);    
                //let the warning presenting 3s then dissolve 
            }
        }
        else if(one_command_line == "wq")//That means save and quit
        {
            ofstream outing((string)filename,ios::out|ios::trunc);
            for(int i = 0; i < Marked_lines.size(); i++)
            {
                outing << Marked_lines[i] << endl;
            } 
            has_saved = 1;
            outing.close();
            fs.close();
            exit(0);
        }
        else if(one_command_line == "q!")
        {
            fs.close();
            exit(0);//whether saved or not ,terminate
        }
        else if(one_command_line.substr(0, 3) == (string)"sub")
        {//code below aims to get the two word for substitution
        //maybe the implement is a little fool :-)
            flag1 = 1;
            if(one_command_line.size() > 3)
            {
                one_command_line = one_command_line.substr(3);
                zeropo = one_command_line.find('\"');
                if(zeropo != string::npos)
                {
                    flag2 = 1;
                    one_command_line = one_command_line.substr(zeropo + 1);
                    firstpo = one_command_line.find('\"');
                    if(firstpo != string::npos)
                    {
                    flag3 = 1; 
                    firstword = one_command_line.substr(0, firstpo); 
                    one_command_line = one_command_line.substr(firstpo + 1);
                    secondpo = one_command_line.find('\"');
                    if(secondpo != string::npos)
                    {
                            flag4 = 1;
                            one_command_line = one_command_line.substr(secondpo + 1);
                            thirdpo = one_command_line.find('\"');
                            if(thirdpo != string::npos)
                            {
                                flag5 = 1;
                                secondword = one_command_line.substr(0, thirdpo);
                            }
                        substitute();
                        Thelines = renew_Thelines(Thelines);
                        REPRINT();
                    }
                    }
                }
            }
        }
        one_command_line.clear();
        //I have to reset the string
        now_command = command.size();
        //I have to position the pointer to the newest position
        mode = normal_mode;
        wmove(File_window, y, x);
        wrefresh(File_window);
    }
    
    //Things below is my extension —————— command history!!
    //It's easy to implement
    else if(in == KEY_UP)
    {
        if(now_command > 0)
        {
            now_command--;
            one_command_line = command[now_command];//search for the last command
            wclear(Command_window);
            getyx(File_window, y, x);
            mvwprintw(Command_window, 0, 0, "%s", (":" + command[now_command]).c_str());
            //I have to translate string into c-string each time or I will get bugs
        }
        return;
    }
    //Things below are just like (and much easier than)insert mode
    else if(in == KEY_DOWN)
    {
        if(now_command < command.size() - 1)
        {
            now_command++;
            one_command_line = command[now_command];
            wclear(Command_window);
            getyx(File_window, y, x);
            mvwprintw(Command_window, 0, 0, "%s", (":" + command[now_command]).c_str());
        }
        return;
    }
    else if(in == KEY_LEFT)
    {
        getyx(Command_window, command_y, command_x);
        if(command_x > 1)
        {
            command_x--;   
        }
        wmove(Command_window, command_y, command_x);
        wrefresh(Command_window);
        return;
    }

    else if(in == KEY_RIGHT)
    {
        getyx(Command_window, command_y, command_x);
        if(command_x <= one_command_line.size() && command_x < COLS - 1)
        {
            command_x++;   
        }
        wmove(Command_window, command_y, command_x);
        wrefresh(Command_window);
        return;
    }

    else if(in == 9)//that means the user type in 'TAB'
    {
        getyx(Command_window, command_y, command_x);
        for(int i = 0; i < 4; i++)
        {
            if(one_command_line.size() == 0)
            {
                one_command_line.push_back(' ');
            }
            else
            {
                one_command_line.insert(one_command_line.begin() + command_x, 1, ' ');
            }
            winsch(Command_window, ' ');
            if(command_x < COLS - 1)
            {
                command_x++;
            }    
            wmove(Command_window, command_y, command_x);
        }
        
        return;
    }

    else if(in == KEY_DC)
    {   
        getyx(Command_window, command_y, command_x);
        mvwdelch(Command_window, command_y, command_x);
        one_command_line.erase(command_x, 1);//I have to erase this character in the string
        return;
    }

    else if(in == KEY_BACKSPACE)
    {
        getyx(Command_window, command_y, command_x);
        if(command_x > 1)
        {
            command_x--;
            mvwdelch(Command_window, command_y, command_x);
            one_command_line.erase(command_x - 1, 1);//I have to erase this character in the string
            wmove(Command_window, command_y, command_x);
        }
        else
        {
            return;
        }
    }

    else//It's similar to insert mode
    {   
        getyx(Command_window, command_y, command_x);
        if(one_command_line.size() == 0)
        {
            one_command_line.push_back(in);
        }
        else
        {
            one_command_line.insert(one_command_line.begin() + command_x - 1, 1, in);
        }
        winsch(Command_window, in);
        if(command_x < COLS - 1)
        {
            command_x++;
        }    
        wmove(Command_window, command_y, command_x);
    }
    wrefresh(Command_window);
}
vector<Line> renew_Thelines(vector<Line> Thelines)
{
    //This function will renew the Thelines vector according to the Mark_lines vector
    //it makes things much easier---I only have to maintain the Mark_lines vector
    //and I can always renew the Thelines by this function and then renew the window
    vector<Line> result;
    has_saved = 0;
    for(int i = 0; i < Marked_lines.size(); i++)
    {
        int howmuchline = ceil(Marked_lines[i].size() *1.0 /COLS);
        //I must use ceil function to make sure that the howmuchline is enough
        if(!howmuchline)
        {
            howmuchline = 1;
        }//Note:this is necessary cuz if howmuchline == 0,then this line is an empty string
        //if we do not let howmuchline = 1 It won't print anything but we know empty string is a string
        //we should print an empty line,especially when user press the enter key
        int sequence_line = 1;
        for(int j = 0; j < howmuchline; j++)
        {    
            result.push_back({Marked_lines[i].substr(COLS * j, COLS), i + 1 , sequence_line});
            //note that i starts from 0 so the indicator must be i + 1
            sequence_line ++;
        } 
    }
    return result;//Then I can assign it to Thelines
}
void REPRINT()
{
    wclear(File_window);
    if(!have_how_much_scroll)
    {
        for(int i = 0; i < Thelines.size(); i++)
        {
            mvwprintw(File_window, i ,0, Thelines[i].oneline.c_str());
        }
    }
    else
    {
        for(int i = have_how_much_scroll; i < LINES - 2 + have_how_much_scroll; i++)
        {
            mvwprintw(File_window, i - have_how_much_scroll, 0, Thelines[i].oneline.c_str());
        }
    }
    //It's convenient to renew the filewindow after I maintain the two vectors
}

void substitute()
{
    for(int i = 0; i < Marked_lines.size(); i++)
    {
        int startpos = 0;
        int index = 0;
        while((index = Marked_lines[i].find(firstword, startpos)) != string::npos)//search for the key word
        //do two things at one time,so cool!
        {
            startpos = index + firstword.size();
            if((startpos >= Marked_lines.size() - 1 || Marked_lines[i][startpos] == ' ') && (index == 0 || Marked_lines[i][index - 1] == ' '))
            {//it means it's a true word and not part of a bigger word
                onesub(index, i);
                startpos -= (firstword.size() - secondword.size());
            }
        }
    }
}
void onesub(int index, int i)
{
    Marked_lines[i].erase(index, firstword.size());//delete the old word
    if(index == 0)
    {
        Marked_lines[i] = secondword + Marked_lines[i];//special case 
    }
    else
    {
        Marked_lines[i] = Marked_lines[i].substr(0, index) + secondword + Marked_lines[i].substr(index);//To insert the new word
    }
}

void Word_completion()
{
    the_whole_words.clear();
    getyx(File_window, y, x);
    Word_window = newwin(1, 100, y + 1, x);
    wrefresh(Word_window);
    wbkgd(Word_window,COLOR_PAIR(Information_color));
    wrefresh(Word_window);
    //I have to fill the 5_word
    int cnt = 0;
    word_x = 0;
    // if(lexicon.find_prefix(one_word) == lexicon.end())
    // {
    //     Word_window = NULL;
    //     REPRINT();
    //     wrefresh(File_window);
    //     wrefresh(Word_window);
    //     return;
    // }
    for(auto i = lexicon.find_prefix(one_word); i != lexicon.end(); ++i)
    {
        if(i.key() == "")
        {
            break;
        }
        the_whole_words.push_back(i.key());
    }
    auto i = lexicon.find_prefix(one_word);
    for(int count1 = 0; count1 < current_browse_word - 1; count1 ++)
    {
        ++i;
    }
    for( ; i != lexicon.end() && cnt < 5; ++i)
    {
        cnt++;
        if(i.key() == "")
        {
            break;
        }
        mvwprintw(Word_window, 0, 20 * (cnt - 1), "%d.%s", cnt, (i.key() + "   ").c_str());
        //  to print the five word
        wrefresh(Word_window);
    }
    // if(the_whole_words[0].size() && in != 9 && in != 32)//32 is for the blank space
    // {
    //     wmove(Word_window, 0, 0);
    // }; + ()indfbrowcurrent_browse_word - 1    auto i =
    // else
    {
        wmove(File_window, y, x);
    }
    wrefresh(Word_window);
    wrefresh(File_window);
}

