#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <regex>
#include <algorithm>
#include <codecvt>
#include <iomanip>

#include "source_text.hpp"

using namespace std;

class MarkovTextGenerator {
private:
    map<string, vector<string>> bigramModel;
    mt19937 rng;

    vector<string> splitWords(const string& text) {
        vector<string> words;
        wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
        wstring wtext = converter.from_bytes(text);

        wregex wordRegex(L"[а-яА-ЯіїєґІЇЄҐ']+");

        auto it = wsregex_iterator(wtext.begin(), wtext.end(), wordRegex);
        auto end = wsregex_iterator();

        for (; it != end; ++it) {
            wstring wword = it->str();
            // знижуємо регістр (для Unicode потрібна локаль)
            locale loc("uk_UA.UTF-8");
            for (auto& ch : wword)
                ch = towlower(ch);

            words.push_back(converter.to_bytes(wword));
        }

        return words;
    }

public:
    MarkovTextGenerator() : rng(random_device{}()) {}

    void buildBigramModel(const string& text) {
        vector<string> words = splitWords(text);
        for (size_t i = 0; i < words.size() - 1; ++i) {
            bigramModel[words[i]].push_back(words[i + 1]);
        }
    }

    string generateText(const string& startWord, size_t maxLength = 200) {
        string currentWord = startWord;
        transform(currentWord.begin(), currentWord.end(), currentWord.begin(), ::tolower);
        
        if (bigramModel.find(currentWord) == bigramModel.end()) {
            return "Помилка: відповідне слово відсутнє у моделі!";
        }

        string result = currentWord;
        for (size_t i = 0; i < maxLength - 1; ++i) {
            const vector<string>& nextWords = bigramModel[currentWord];
            if (nextWords.empty()) break;
            
            uniform_int_distribution<size_t> dist(0, nextWords.size() - 1);
            currentWord = nextWords[dist(rng)];
            result += " " + currentWord;
        }
        return result;
    }

    void saveToHTML(const string& startWord, const string& generatedText) {
        ofstream htmlFile("generated_page.html");
        if (!htmlFile.is_open()) {
            cerr << "Помилка при створенні HTML файлу" << endl;
            return;
        }

        htmlFile << R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Згенерований текст</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <h1>Тема: Штучний інтелект сьогодні</h1>
    <h2>Початкове слово: <i>)" << startWord << R"(</i></h2>
    <p>)" << generatedText << R"(</p>
    <h2>Матриця біграм</h2>
    <table>
        <tr>
            <th></th>)";

        // Заголовки таблиці
        for (const auto& pair : bigramModel) {
            htmlFile << "<th>" << pair.first << "</th>";
        }
        htmlFile << "</tr>";

        // Дані таблиці
        for (const auto& row : bigramModel) {
            htmlFile << "<tr><th>" << row.first << "</th>";
            for (const auto& col : bigramModel) {
                size_t count = count_if(row.second.begin(), row.second.end(),
                    [&col](const string& word) { return word == col.first; });
                double probability = row.second.empty() ? 0.0 : 
                    static_cast<double>(count) / row.second.size();
                htmlFile << "<td>" << fixed << setprecision(2) << probability << "</td>";
            }
            htmlFile << "</tr>";
        }

        htmlFile << R"(
    </table>
</body>
</html>)";

        htmlFile.close();
        cout << "HTML файл успішно створено: generated_page.html" << endl;
    }
};

int main() {
    MarkovTextGenerator generator;
    generator.buildBigramModel(SOURCE_TEXT);

    string startWord;
    cout << "Введіть початкове слово: ";
    cin >> startWord;

    string generatedText = generator.generateText(startWord);
    cout << "\nЗгенерований текст:\n" << generatedText << endl;

    generator.saveToHTML(startWord, generatedText);
    return 0;
} 