# UNO-game_CSC3002
A console-based UNOcard game implemented in C++. Allow 1 player and 3 bots. Original designed character system and new function card.

<img width="2259" height="1696" alt="屏幕截图 2026-03-26 175514" src="https://github.com/user-attachments/assets/e412052b-a25e-429c-b8fa-398de33b2d51" />

This project was developed as a group assignment for course CSC3002 at CUHK-ShenZhen.I was responsible for implementing the card functionality system and integrating it with the game engine in this UNO game project. And I also took responsibility for debugging and testing.

# How to Play
Compile the game:
g++ -std=c++17 -o uno.exe main.cpp gameengine.cpp card.cpp player.cpp unoui.cpp -I.


Run the game:
.\uno.exe

Follow the on‑screen prompts

You’ll be shown your current hand, use A/D to choose the card you want to play, W to draw from deck, s to use your character skill

