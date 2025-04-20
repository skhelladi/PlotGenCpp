#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <functional>
#include <algorithm>
#include <iostream>

// Forward declaration for stb_image_write
namespace stb {
    int stbi_write_jpg(char const *filename, int w, int h, int comp, const void *data, int quality);
    int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
}

class PlotGen {
public:
    struct Style {
        sf::Color color;
        float thickness;
        std::string line_style;
        std::string legend;
        std::string symbol_type; // "none", "circle", "square", "triangle", "diamond", "star"
        float symbol_size;      // Symbol size in pixels

        Style(
            sf::Color color_ = sf::Color::Blue,
            float thickness_ = 2.0f,
            const std::string& line_style_ = "solid",
            const std::string& legend_ = "",
            const std::string& symbol_type_ = "none",
            float symbol_size_ = 6.0f
        );
    };

    struct Figure {
        std::string title, xlabel, ylabel;
        float xmin = -10, xmax = 10, ymin = -10, ymax = 10;
        bool show_leg = true;
        bool show_major_grid = false; // Option to display major grid
        bool show_minor_grid = false; // Option to display minor grid
        sf::Color major_grid_color = sf::Color(200, 200, 200); // Light gray for major grid
        sf::Color minor_grid_color = sf::Color(230, 230, 230); // Very light gray for minor grid
        bool is_polar = false; // Indicates if the graph is in polar coordinates
        bool equal_axes = false; // Option for axes of the same dimension
        struct Curve {
            std::vector<float> x, y;
            Style style;
            float bar_width_ratio = 0.9f; // Field to store width ratio
        };
        std::vector<Curve> curves;
        std::vector<std::string> curve_types;
    };

    // Constructor
    PlotGen(unsigned int width = 1200, unsigned int height = 900, unsigned int rows = 1, unsigned int cols = 1);

    // Add a figure at position (row, col)
    Figure& subplot(unsigned int row, unsigned int col);

    // Figure configuration methods
    void set_title(Figure& fig, const std::string& title);
    void set_xlabel(Figure& fig, const std::string& label);
    void set_ylabel(Figure& fig, const std::string& label);
    void set_axis_limits(Figure& fig, float xmin, float xmax, float ymin, float ymax);
    void set_polar_axis_limits(Figure& fig, float max_radius);
    void show_legend(Figure& fig, bool show);
    void grid(Figure& fig, bool major = true, bool minor = false);
    void set_grid_color(Figure& fig, sf::Color major_color, sf::Color minor_color);
    void set_equal_axes(Figure& fig, bool equal = true);

    // 2D curve plotting
    void plot(Figure& fig, const std::vector<float>& x, const std::vector<float>& y, const Style& style = Style());

    // Histogram
    void hist(Figure& fig, const std::vector<float>& data, int bins = 10, const Style& style = Style(), float bar_width_ratio = 0.9f);

    // Polar plot
    void polar_plot(Figure& fig, const std::vector<float>& theta, const std::vector<float>& r, const Style& style = Style());

    // Display and render
    void show();

    // Save to file
    void save(const std::string& filename);

private:
    sf::RenderWindow window;
    sf::RenderTexture texture;
    sf::Sprite sprite;
    sf::Font font;
    unsigned int width, height, rows, cols;
    std::vector<Figure> figures;

    // Special character symbols
    std::string degree_symbol = "\u00B0"; // Degree symbol (°)
    std::string pi_symbol = "\u03C0";     // Pi symbol (π)

    // Rendering methods
    void render();
    void draw_axes(const Figure& fig, float w, float h);
    void draw_grid(const Figure& fig, float w, float h);
    void draw_polar_grid(const Figure& fig, float w, float h);
    void draw_curve(const Figure& fig, const Figure::Curve& curve, float w, float h);
    void draw_histogram(const Figure& fig, const Figure::Curve& curve, float w, float h);
    sf::Color getColorFromHeight(float height);
    sf::Vector2f to_screen(const Figure& fig, float x, float y, float w, float h) const;
    void draw_text(const Figure& fig, float w, float h);
    void draw_symbol(const sf::Vector2f& position, const std::string& symbol_type, float size, const sf::Color& color);
};

