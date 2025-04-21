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
        double thickness;
        std::string line_style;
        std::string legend;
        std::string symbol_type; // "none", "circle", "square", "triangle", "diamond", "star"
        double symbol_size;      // Symbol size in pixels

        Style(
            sf::Color color_ = sf::Color::Blue,
            double thickness_ = 2.0f,
            const std::string& line_style_ = "solid",
            const std::string& legend_ = "",
            const std::string& symbol_type_ = "none",
            double symbol_size_ = 6.0f
        );
    };

    struct Figure {
        std::string title, xlabel, ylabel;
        double xmin = -10, xmax = 10, ymin = -10, ymax = 10;
        bool show_leg = true;
        // Positions possibles de la légende: "top-right", "top-left", "bottom-right", "bottom-left", "outside-right"
        std::string legend_position = "top-right";
        bool show_major_grid = false; // Option to display major grid
        bool show_minor_grid = false; // Option to display minor grid
        sf::Color major_grid_color = sf::Color(200, 200, 200); // Light gray for major grid
        sf::Color minor_grid_color = sf::Color(230, 230, 230); // Very light gray for minor grid
        bool is_polar = false; // Indicates if the graph is in polar coordinates
        bool equal_axes = false; // Option for axes of the same dimension
        struct Curve {
            std::vector<double> x, y;
            Style style;
            double bar_width_ratio = 0.9f; // Field to store width ratio
            std::string text_content; // For storing text to display at a position
            double head_size = 10.0;  // For storing arrow head size
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
    void set_axis_limits(Figure& fig, double xmin, double xmax, double ymin, double ymax);
    void set_polar_axis_limits(Figure& fig, double max_radius);
    void show_legend(Figure& fig, bool show);
    void grid(Figure& fig, bool major = true, bool minor = false);
    void set_grid_color(Figure& fig, sf::Color major_color, sf::Color minor_color);
    void set_equal_axes(Figure& fig, bool equal = true);
    void set_legend_position(Figure& fig, const std::string& position);

    // 2D curve plotting
    void plot(Figure& fig, const std::vector<double>& x, const std::vector<double>& y, const Style& style = Style());

    // Histogram
    void hist(Figure& fig, const std::vector<double>& data, int bins = 10, const Style& style = Style(), double bar_width_ratio = 0.9f);

    // Polar plot
    void polar_plot(Figure& fig, const std::vector<double>& theta, const std::vector<double>& r, const Style& style = Style());

    // Circle with center (x0, y0) and radius r
    void circle(Figure& fig, double x0, double y0, double r, const Style& style = Style());

    // Text at position (x, y)
    void text(Figure& fig, double x, double y, const std::string& text_content, const Style& style = Style());
    
    // Arrow from (x1, y1) to (x2, y2)
    void arrow(Figure& fig, double x1, double y1, double x2, double y2, const Style& style = Style(), double head_size = 10.0);
    
    // Line from (x1, y1) to (x2, y2)
    void line(Figure& fig, double x1, double y1, double x2, double y2, const Style& style = Style());
    
    // Arc centered at (x0, y0) from angle1 to angle2 (in radians) with radius r
    void arc(Figure& fig, double x0, double y0, double r, double angle1, double angle2, const Style& style = Style(), int num_points = 50);

    // Bezier curve with control points (x0,y0), (x1,y1), (x2,y2), (x3,y3)
    void bezier(Figure& fig, double x0, double y0, double x1, double y1, 
                double x2, double y2, double x3, double y3, 
                const Style& style = Style(), int num_points = 100);

    // Bezier curve with control points
    void bezier(Figure& fig, const std::vector<double>& x, const std::vector<double>& y, 
                const Style& style = Style(), int num_points = 100);

    // Natural cubic spline through points
    void spline(Figure& fig, const std::vector<double>& x, const std::vector<double>& y, 
                const Style& style = Style(), int num_points = 100);
                
    // Cardinal spline through points with tension parameter
    void cardinal_spline(Figure& fig, const std::vector<double>& x, const std::vector<double>& y, 
                         double tension = 0.5, const Style& style = Style(), int num_points = 100);

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
    void draw_axes(const Figure& fig, double w, double h);
    void draw_grid(const Figure& fig, double w, double h);
    void draw_polar_grid(const Figure& fig, double w, double h);
    void draw_curve(const Figure& fig, const Figure::Curve& curve, double w, double h);
    void draw_histogram(const Figure& fig, const Figure::Curve& curve, double w, double h);
    sf::Color getColorFromHeight(double height);
    sf::Vector2f to_screen(const Figure& fig, double x, double y, double w, double h) const;
    void draw_text(const Figure& fig, double w, double h);
    void draw_text(const Figure& fig, const Figure::Curve& curve, double w, double h);
    void draw_arrow_head(const Figure& fig, const Figure::Curve& curve, double w, double h);
    void draw_symbol(const sf::Vector2f& position, const std::string& symbol_type, double size, const sf::Color& color);
};

