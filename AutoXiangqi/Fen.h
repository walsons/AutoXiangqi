#ifndef  FEN_H
#define FEN_H

#include <Windows.h>
#include <string>

class Fen
{
public:
    Fen(const std::string& fenString)
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
            }
        }
        color = fenString[fenString.size() - std::string("w - - 0 1").size()];
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
        return fenSegment + " " + color + " - - 0 1";
    }

    char Color()
    {
        return color;
    }

    std::string operator-(const Fen& fen)
    {
        POINT from = { 0, 0 }, to{ 0, 0 };
        for (int i = 0; i < 10; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                if ((fen.b[i][j] != '0') && b[i][j] == '0')
                    from = { i, j };
                else if (fen.b[i][j] != b[i][j])
                    to = { i, j };
            }
        }

        std::string move;
        move.push_back(from.x + 'a');
        move.push_back((9 - from.y) + '0');
        move.push_back(to.x + 'a');
        move.push_back((9 - to.y) + '0');
        return move;
    }

    bool operator==(const Fen& fen)
    {
        return Get() == fen.Get();
    }

    Fen& push(const std::string& move)
    {
        POINT from = { 0, 0 }, to{ 0, 0 };
        from.x = move[0] - 'a';
        from.y = 9 - (move[1] - '0');
        to.x = move[2] - 'a';
        to.y = 9 - (move[3] - '0');
        b[to.y][to.x] = b[from.y][from.x];
        b[from.y][from.x] = '0';
        color = (color == 'w' ? 'b' : 'w');
        return *this;
    }
private:
    // '0' delegate empty blank grid for convenience
    char b[10][9];
    char color;
};


#endif
