#ifndef  FEN_H
#define FEN_H

#include <Windows.h>
#include <string>
#include <vector>

class Fen
{
public:
    Fen(const std::string& fenString = "4r4/9/9/9/9/9/9/9/9/9 w - - 0 1")
    {
        std::string boardString;
        for (auto c : fenString)
        {
            if (c == ' ')
                break;
            else if (c > '0' && c <= '9')
                boardString += std::string((c - '0'), '0');
            else if (c == '/')
                continue;
            else
                boardString += c;
        }
        int index = 0;
        for (int i = 0; i < 10; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                b[i][j] = boardString[index++];
                real[i][j] = b[i][j];
            }
        }
        color = fenString[fenString.size() - std::string("w - - 0 1").size()];
    }

    Fen(const Fen& fen)
    {
        m_Moves = fen.m_Moves;
        color = fen.color;
        for (int i = 0; i < 10; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                b[i][j] = fen.b[i][j];
                real[i][j] = fen.real[i][j];
            }
        }
    }

    std::string Get() const
    {
        std::string fenSegment;
        for (int i = 0; i < 10; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                if (b[i][j] != '0')
                    fenSegment += b[i][j];
                else
                {
                    if (fenSegment.empty() || (fenSegment.back() < '0' || fenSegment.back() >= '9'))
                        fenSegment += "1";
                    else
                        fenSegment.back() = fenSegment.back() + 1;
                }
            }
            fenSegment += "/";
        }
        fenSegment.pop_back();
        auto fenColor = color;
        if (m_Moves.size() % 2)
            fenColor = (fenColor == 'w' ? 'b' : 'w');
        fenSegment += std::string(" ") + fenColor + " - - 0 1";
        if (!m_Moves.empty())
        {
            fenSegment += " moves";
            for (auto step : m_Moves)
            {
                fenSegment += (" " + step);
            }
        }
        return fenSegment;
    }

    std::string GetReal() const
    {
        std::string fenSegment;
        for (int i = 0; i < 10; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                if (real[i][j] != '0')
                    fenSegment += real[i][j];
                else
                {
                    if (fenSegment.empty() || (fenSegment.back() < '0' || fenSegment.back() >= '9'))
                        fenSegment += "1";
                    else
                        fenSegment.back() = fenSegment.back() + 1;
                }
            }
            fenSegment += "/";
        }
        fenSegment.pop_back();
        return fenSegment + " " + color + " - - 0 1";
    }

    char Color()
    {
        return color;
    }

    std::string operator-(const Fen& fen)
    {
        POINT from = { 0, 0 }, to{ 0, 0 };
        bool flag1 = false, flag2 = false;
        for (int i = 0; i < 10; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                if ((fen.real[i][j] != '0') && real[i][j] == '0')
                {
                    flag1 = true;
                    from = { i, j };
                }
                else if (fen.real[i][j] != real[i][j])
                {
                    flag2 = true;
                    to = { i, j };
                }
            }
        }
        if (!flag1 || !flag2)
            return "invalid";
        std::string move;
        move.push_back(from.y + 'a');
        move.push_back((9 - from.x) + '0');
        move.push_back(to.y + 'a');
        move.push_back((9 - to.x) + '0');
        return move;
    }

    bool operator==(const Fen& fen)
    {
        return GetReal() == fen.GetReal();
    }

    int FenSymbol(const std::string& fen)
    {
        auto pos = fen.find_first_of(' ');
        auto fenSegment = fen.substr(0, pos);
        int symbol = 0;
        for (auto c : fenSegment)
        {
            switch (c)
            {
            case 'r':
                symbol += (1 << 13);
                break;
            case 'n':
                symbol += (1 << 12);
                break;
            case 'b':
                symbol += (1 << 11);
                break;
            case 'a':
                symbol += (1 << 10);
                break;
            case 'k':
                symbol += (1 << 9);
                break;
            case 'c':
                symbol += (1 << 8);
                break;
            case 'p':
                symbol += (1 << 7);
                break;
            case 'R':
                symbol += (1 << 6);
                break;
            case 'N':
                symbol += (1 << 5);
                break;
            case 'B':
                symbol += (1 << 4);
                break;
            case 'A':
                symbol += (1 << 3);
                break;
            case 'K':
                symbol += (1 << 2);
                break;
            case 'C':
                symbol += (1 << 1);
                break;
            case 'P':
                symbol += (1 << 0);
                break;
            default:
                break;
            }
        }
        return symbol;
    }

    Fen& push(const std::string& move)
    {
        POINT from = { 0, 0 }, to{ 0, 0 };
        from.x = move[0] - 'a';
        from.y = 9 - (move[1] - '0');
        to.x = move[2] - 'a';
        to.y = 9 - (move[3] - '0');
        real[to.y][to.x] = real[from.y][from.x];
        real[from.y][from.x] = '0';
        //color = (color == 'w' ? 'b' : 'w');
        if (FenSymbol(GetReal()) == FenSymbol(Get()))
        {
            m_Moves.push_back(move);
        }
        else
        {
            for (int i = 0; i < 10; ++i)
            {
                for (int j = 0; j < 9; ++j)
                {
                    b[i][j] = real[i][j];
                }
            }
            m_Moves.clear();
        }
        return *this;
    }

public:
    std::vector<std::string> m_Moves;

private:
    // '0' delegate empty blank grid for convenience
    char b[10][9];
    char real[10][9];
    char color;
};


#endif
