
//
// Disclaimer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// Note that the "Run Script" build phase will copy the required frameworks
// or dylibs to your application bundle so you can execute it on any OS X
// computer.
//
// Your resource files (images, sounds, fonts, ...) are also copied to your
// application bundle. To get the path to these resources, use the helper
// function `resourcePath()` from ResourcePath.hpp
//

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>

// Here is a small helper for you! Have a look.
#include "ResourcePath.hpp"

void game_loop(sf::RenderWindow &window,
               std::function<void(bool)> onUpdate,
               std::function<void(void)> onRender)
{

    // Start the game loop
    while (window.isOpen())
    {
        bool space = false;
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            // Escape pressed: exit
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
            {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                space = true;
            }
        }

        // Update state
        // std::cout << "before update" << std::endl;
        onUpdate(space);
        // std::cout << "after update" << std::endl;

        // Update the window
        // std::cout << "before render" << std::endl;
        window.clear();
        onRender();
        window.display();
        // std::cout << "after render" << std::endl;
    }
}

int main(int, char const **)
{
    const sf::Vector2f windowSize(1280.f, 1024.f);
    const int rowCount = 200;
    const int columnCount = 200;
    const int cellCount = rowCount * columnCount;
    const auto cellColor = sf::Color::Red;

    // Create the main window
    sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "SFML window");

    // Set the Icon
    sf::Image icon;
    if (!icon.loadFromFile(resourcePath() + "icon.png"))
    {
        return EXIT_FAILURE;
    }
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    std::vector<bool>
        cells(cellCount);

    const auto generate = [&cells] {
        std::srand(time(0));
        std::generate(cells.begin(), cells.end(), [] {
            return std::rand() % 2;
        });
    };
    generate();

    std::vector<sf::Color const *> colors(cellCount);

    const auto getCell = [&cells](int x, int y) {
        return cells[x + y * columnCount];
    };

    const auto countNeighbours = [&cells, &getCell](int x, int y) {
        int count = 0;
        for (int dy = -1; dy < 2; ++dy)
        {
            for (int dx = -1; dx < 2; ++dx)
            {
                if (dx == 0 && dy == 0)
                {
                    continue;
                }
                const int nx = x + dx, ny = y + dy;
                if (nx < 0 || nx >= columnCount || ny < 0 || ny >= rowCount)
                {
                    continue;
                }
                if (getCell(nx, ny))
                {
                    ++count;
                }
            }
        }
        return count;
    };

    const sf::Vector2f brushSize(windowSize.x / columnCount, windowSize.y / rowCount);
    sf::RectangleShape cellBrush(brushSize);
    std::cout << "brushSize: "
              << brushSize.x << ','
              << brushSize.y << std::endl;
    cellBrush.setFillColor(cellColor);

    int frameCount = 0;
    int lastAlive = 0;
    game_loop(window,
              [&](bool spacePressed) {
                  if (spacePressed)
                  {
                      generate();
                      return;
                  }

                  std::vector<bool> updatedCells(cells);
                  int born = 0, died = 0;
                  for (int row = 0; row < rowCount; ++row)
                  {
                      for (int column = 0; column < columnCount; ++column)
                      {
                          const int cellIndex = column + row * columnCount;
                          const auto neighbourCount = countNeighbours(column, row);
                          // set color according to neighbours
                          switch (neighbourCount)
                          {
                          case 2:
                          case 3:
                              colors[cellIndex] = &sf::Color::Green;
                              break;
                          default:
                              colors[cellIndex] = &sf::Color::Red;
                              break;
                          }

                          const bool isAlive = getCell(column, row);
                          if (isAlive)
                          {
                              // Any live cell with fewer than two live neighbors dies, as if by under population.
                              // Any live cell with two or three live neighbors lives on to the next generation.
                              // Any live cell with more than three live neighbors dies, as if by overpopulation.
                              if (neighbourCount < 2 || neighbourCount > 3)
                              {
                                  updatedCells[column + row * columnCount] = false;
                                  ++died;
                              }
                          }
                          else
                          {
                              // Any dead cell with exactly three live neighbors becomes a live cell, as if by reproduction.
                              if (neighbourCount == 3)
                              {
                                  updatedCells[column + row * columnCount] = true;
                                  ++born;
                              }
                          }
                      }
                  }
                  //   std::cout
                  //       << "born: " << born << " - "
                  //       << "died: " << died << std::endl;

                  cells = updatedCells;
              },
              [&] {
                  for (int row = 0; row < rowCount; ++row)
                  {
                      for (int column = 0; column < columnCount; ++column)
                      {
                          const int cellIndex = column + row * columnCount;
                          if (!getCell(column, row))
                          {
                              continue;
                          }

                          const float pos_x = brushSize.x * column;
                          const float pos_y = brushSize.y * row;
                          cellBrush.setPosition(pos_x, pos_y);
                          cellBrush.setFillColor(*colors[cellIndex]);
                          window.draw(cellBrush);
                      }
                  }
              });

    return EXIT_SUCCESS;
}
