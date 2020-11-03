#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <fstream>

#include "json.hpp"

using namespace std;
using json = nlohmann::json;

enum Choice
{
    NONE,
    ROCK,
    PAPER,
    SCISSORS
};

class Player
{
public:
    Player() : choice1(Choice::NONE), choice2(Choice::NONE),
               prevChoice1(Choice::NONE), isGameOver(false) {}

    void Close()
    {
        isGameOver = true;
        cv1.notify_one();
        cv2.notify_one();
    }

    ~Player()
    {
        Close();
    }

    void GenerateChoice1()
    {
        unique_lock<mutex> l(m1);
        while (!isGameOver)
        {
            prevChoice1 = choice1;
            choice1 = (Choice)(rand() % 3 + 1);
            cv1.wait(l);
        }
    }

    Choice GetChoice1()
    {
        cv2.notify_one();
        this_thread::sleep_for(chrono::microseconds(500));
        return choice1;
    }

    void GenerateChoice2()
    {
        unique_lock<mutex> l(m2);
        while (!isGameOver)
        {
            if (prevChoice1 == Choice::NONE)
            {
                choice2 = (Choice)(rand() % 3 + 1);
            } else {
                choice2 = prevChoice1;
            }
            cv2.wait(l);
        }
    }

    Choice GetChoice2()
    {
        cv1.notify_one();
        this_thread::sleep_for(chrono::microseconds(500));
        return choice2;
    }

private:
    Choice choice1;
    Choice choice2;
    Choice prevChoice1;
    bool isGameOver;
    condition_variable cv1;
    mutex m1;
    condition_variable cv2;
    mutex m2;
};

string ParseChoice(Choice ch)
{
    if (ch == Choice::PAPER)
    {
        return "paper";
    } else if (ch == Choice::ROCK) {
        return "rock";
    } else if (ch == Choice::SCISSORS) {
        return "scissors";
    } else {
        return "";
    }
}

string FindWinner(Choice ch1, Choice ch2){
    if (ch1 == ch2) {
        return "null";
    } else if(ch1 == Choice::ROCK){
        if(ch2 == Choice::PAPER){
            return "Player2";
        }else{
            return "Player1";
        } 
    } else if(ch1 == Choice::PAPER){
        if(ch2 == Choice::SCISSORS){
            return "Player2";
        }else{
            return "Player1";
        }
    } else if(ch1 == Choice::SCISSORS){
        if(ch2 == Choice::ROCK){
            return "PLayer2";
        }else{
            return "Player1";
        }
    }
}

int main()
{
    srand(time(0));
    {
        Player p;
        thread t1(&Player::GenerateChoice1, &p);
        thread t2(&Player::GenerateChoice2, &p);
        ofstream of("result.json");
        for (int i = 0; i < 100; i++)
        {
            Choice ch1 = p.GetChoice1();
            Choice ch2 = p.GetChoice2();
            json j;
            j["Round"] = i + 1;
            j["Winner"] = FindWinner(ch1, ch2);
            j["Inputs"] = {{"Player1", ParseChoice(ch1)}, {"Player2", ParseChoice(ch2)}};
            of << setw(4) << j << "," << endl;
        }

        p.Close();
        t1.join();
        t2.join();
    }
    cout<<"Results are written to the file results.json"<<endl;
    return 1;
}