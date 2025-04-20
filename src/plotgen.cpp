#include "plotgen.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <stdexcept>

// Define STB_IMAGE_WRITE_IMPLEMENTATION before including stb_image_write.h
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb/stb_image_write.h"

// Style struct constructor implementation
PlotGen::Style::Style(
    sf::Color color_, 
    float thickness_, 
    const std::string& line_style_, 
    const std::string& legend_, 
    const std::string& symbol_type_, 
    float symbol_size_
) : color(color_), thickness(thickness_), line_style(line_style_), 
    legend(legend_), symbol_type(symbol_type_), symbol_size(symbol_size_)
{
}

// Constructor
PlotGen::PlotGen(unsigned int width, unsigned int height, unsigned int rows, unsigned int cols) 
    : window(sf::VideoMode(width, height), "PlotGen", sf::Style::Default), width(width), height(height), rows(rows), cols(cols) {
    texture.create(width, height);
    sprite.setTexture(texture.getTexture());
    
    // Search for the font in several possible locations
    if (font.loadFromFile("fonts/arial.ttf")) {
        std::cout << "Font loaded from current directory" << std::endl;
    } else 
    if (font.loadFromFile("build/arial.ttf")) {
        std::cout << "Font loaded from build directory" << std::endl;
    } else if (font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
        std::cout << "LiberationSans font loaded" << std::endl;
    } else if (font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cout << "DejaVuSans font loaded" << std::endl;
    } else {
        std::cerr << "WARNING: Unable to load a font compatible with Unicode characters" << std::endl;
        throw std::runtime_error("Unable to load a font supporting Unicode");
    }
    
    figures.resize(rows * cols);
    for (auto& fig : figures) {
        fig.curve_types.clear();
        fig.curves.clear();
    }
    
    // Enable antialiasing to improve graphics quality
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    window.create(sf::VideoMode(width, height), "PlotGen", sf::Style::Default, settings);
    
    // Configure texture with better quality
    texture.create(width, height);
    texture.setSmooth(true);
    sprite.setTexture(texture.getTexture());
}

// Add a figure at a position (row, col)
PlotGen::Figure& PlotGen::subplot(unsigned int row, unsigned int col) {
    if (row >= rows || col >= cols) {
        throw std::out_of_range("Subplot index out of range");
    }
    return figures[row * cols + col];
}

// Figure configuration
void PlotGen::set_title(Figure& fig, const std::string& title) { fig.title = title; }
void PlotGen::set_xlabel(Figure& fig, const std::string& label) { fig.xlabel = label; }
void PlotGen::set_ylabel(Figure& fig, const std::string& label) { fig.ylabel = label; }
void PlotGen::set_axis_limits(Figure& fig, float xmin, float xmax, float ymin, float ymax) {
    fig.xmin = xmin; fig.xmax = xmax;
    fig.ymin = ymin; fig.ymax = ymax;
}

// New function specific for polar graphs ensuring axes of the same dimension
void PlotGen::set_polar_axis_limits(Figure& fig, float max_radius) {
    // For a polar graph, X and Y axes must have the same scale
    // and be centered on (0,0)
    fig.is_polar = true; // Mark as a polar graph
    fig.xmin = -max_radius;
    fig.xmax = max_radius;
    fig.ymin = -max_radius;
    fig.ymax = max_radius;
}

void PlotGen::show_legend(Figure& fig, bool show) { fig.show_leg = show; }

// Methods to enable/disable grids
void PlotGen::grid(Figure& fig, bool major, bool minor) {
    fig.show_major_grid = major;
    fig.show_minor_grid = minor;
}

void PlotGen::set_grid_color(Figure& fig, sf::Color major_color, sf::Color minor_color) {
    fig.major_grid_color = major_color;
    fig.minor_grid_color = minor_color;
}

void PlotGen::set_equal_axes(Figure& fig, bool equal) { fig.equal_axes = equal; }

// 2D curve plotting
void PlotGen::plot(Figure& fig, const std::vector<float>& x, const std::vector<float>& y, const Style& style) {
    if (x.size() != y.size() || x.empty()) {
        throw std::invalid_argument("x and y vectors must have the same size and not be empty");
    }
    
    fig.curves.push_back({x, y, style});
    fig.curve_types.push_back("2D");
}

// Histogram
void PlotGen::hist(Figure& fig, const std::vector<float>& data, int bins, const Style& style, float bar_width_ratio) {
    if (data.empty()) {
        throw std::invalid_argument("data vector must not be empty");
    }
    
    std::vector<float> hist_x, hist_y;
    float min_val = *std::min_element(data.begin(), data.end());
    float max_val = *std::max_element(data.begin(), data.end());
    float bin_width = (max_val - min_val) / bins;

    std::vector<int> counts(bins, 0);
    for (float val : data) {
        int bin = std::min(static_cast<int>((val - min_val) / bin_width), bins - 1);
        counts[bin]++;
    }

    for (int i = 0; i < bins; ++i) {
        hist_x.push_back(min_val + i * bin_width);
        hist_y.push_back(static_cast<float>(counts[i]));
    }
    
    // Automatically set axis limits if they haven't been specified
    // Check if default limits are being used
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);
    
    if (using_default_limits) {
        // Set X limits to include all bars with a 5% margin
        float x_margin = (max_val - min_val) * 0.05f;
        fig.xmin = min_val - x_margin;
        fig.xmax = max_val + bin_width + x_margin; // Add bin_width to see the last bar completely
        
        // Set Y limits to include all bars with a 10% margin
        float max_count = *std::max_element(hist_y.begin(), hist_y.end());
        float y_margin = max_count * 0.1f;
        fig.ymin = 0; // Histograms generally start at 0
        fig.ymax = max_count + y_margin;
    }
    
    // Create a curve with the specified bar width ratio
    Figure::Curve curve = {hist_x, hist_y, style};
    curve.bar_width_ratio = bar_width_ratio; // Apply bar width ratio
    
    fig.curves.push_back(curve);
    fig.curve_types.push_back("HIST");
}

// Polar plot
void PlotGen::polar_plot(Figure& fig, const std::vector<float>& theta, const std::vector<float>& r, const Style& style) {
    if (theta.size() != r.size() || theta.empty()) {
        throw std::invalid_argument("theta and r vectors must have the same size and not be empty");
    }
    
    // Mark the figure as being in polar coordinates
    fig.is_polar = true;
    
    // Find the maximum radius to adapt the graph limits
    float max_r = *std::max_element(r.begin(), r.end());
    
    // Check if we're using default limits
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);
    
    // If the limits haven't been explicitly set or are default, calculate them automatically
    if (using_default_limits) {
        // Add a 10% margin for better visualization
        set_polar_axis_limits(fig, max_r * 1.1f);
    } else {
        // Check if current limits are sufficient to contain all data
        // and if they respect polar graph constraints (axes of same dimension)
        float current_range_x = (fig.xmax - fig.xmin) / 2;
        float current_range_y = (fig.ymax - fig.ymin) / 2;
        float needed_range = max_r * 1.1f;
        
        // Ensure axes have the same scale
        float max_range = std::max(current_range_x, current_range_y);
        
        if (needed_range > max_range || std::abs(current_range_x - current_range_y) > 1e-6) {
            // Use set_polar_axis_limits to ensure axes of same dimension
            float max_limit = std::max(needed_range, max_range);
            set_polar_axis_limits(fig, max_limit);
        }
    }
    
    // Convert to Cartesian coordinates for display
    std::vector<float> x, y;
    for (size_t i = 0; i < theta.size(); ++i) {
        x.push_back(r[i] * std::cos(theta[i]));
        y.push_back(r[i] * std::sin(theta[i]));
    }
    
    fig.curves.push_back({x, y, style});
    fig.curve_types.push_back("POLAR");
}

// Display and render
void PlotGen::show() {
    // Perform initial rendering
    render();
    
    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }
        }

        // Render and display (no need to do it every iteration if nothing changes)
        window.clear(sf::Color::White);
        window.draw(sprite);
        window.display();
    }
}

// Save
void PlotGen::save(const std::string& filename) {
    render(); // Ensure the rendering is up to date
    
    sf::Image screenshot = texture.getTexture().copyToImage();
    
    if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".png") == 0) {
        if (!screenshot.saveToFile(filename)) {
            throw std::runtime_error("Unable to save image in PNG format");
        }
    } else if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".jpg") == 0) {
        // Conversion for stb_image_write with high quality
        std::vector<unsigned char> pixels(width * height * 3); // RGB
        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                sf::Color c = screenshot.getPixel(x, y);
                size_t index = (y * width + x) * 3;
                pixels[index] = c.r;
                pixels[index + 1] = c.g;
                pixels[index + 2] = c.b;
            }
        }
        
        // Write JPG image with 95% quality (better quality)
        if (!stbi_write_jpg(filename.c_str(), width, height, 3, pixels.data(), 95)) {
            throw std::runtime_error("Unable to save image in JPG format");
        }
    } else {
        throw std::invalid_argument("File format not supported. Use .png or .jpg");
    }
    
    std::cout << "Image saved to: " << filename << " (Dimensions: " << width << "x" << height << ")" << std::endl;
}

void PlotGen::render() {
    texture.clear(sf::Color::White);
    
    // Go through all subplots
    for (unsigned int row = 0; row < rows; ++row) {
        for (unsigned int col = 0; col < cols; ++col) {
            Figure& fig = figures[row * cols + col];
            
            // Define the view for this subplot
            float subplot_width = static_cast<float>(width) / cols;
            float subplot_height = static_cast<float>(height) / rows;
            
            // For polar graphs, ensure dimensions are square
            if (fig.is_polar) {
                float min_size = std::min(subplot_width, subplot_height);
                float x_offset = (subplot_width - min_size) / 2.0f;
                float y_offset = (subplot_height - min_size) / 2.0f;
                
                sf::View view(sf::FloatRect(0, 0, min_size, min_size));
                view.setViewport(sf::FloatRect(
                    static_cast<float>(col) / cols + x_offset / width,
                    static_cast<float>(row) / rows + y_offset / height,
                    min_size / width,
                    min_size / height
                ));
                texture.setView(view);
            } else if (fig.equal_axes) {
                float min_size = std::min(subplot_width, subplot_height);
                float x_offset = (subplot_width - min_size) / 2.0f;
                float y_offset = (subplot_height - min_size) / 2.0f;
                
                sf::View view(sf::FloatRect(0, 0, min_size, min_size));
                view.setViewport(sf::FloatRect(
                    static_cast<float>(col) / cols + x_offset / width,
                    static_cast<float>(row) / rows + y_offset / height,
                    min_size / width,
                    min_size / height
                ));
                texture.setView(view);
            } else {
                sf::View view(sf::FloatRect(0, 0, subplot_width, subplot_height));
                view.setViewport(sf::FloatRect(
                    static_cast<float>(col) / cols,
                    static_cast<float>(row) / rows,
                    1.0f / cols,
                    1.0f / rows
                ));
                texture.setView(view);
            }
            
            // Draw the subplot frame
            sf::RectangleShape frame;
            if (fig.is_polar || fig.equal_axes) {
                float min_size = std::min(subplot_width, subplot_height);
                frame.setSize(sf::Vector2f(min_size, min_size));
            } else {
                frame.setSize(sf::Vector2f(subplot_width, subplot_height));
            }
            frame.setFillColor(sf::Color::Transparent);
            frame.setOutlineColor(sf::Color::Black);
            frame.setOutlineThickness(1.0f);
            texture.draw(frame);

            // Draw axes
            draw_axes(fig, fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width, 
                      fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);

            // Draw curves
            for (size_t i = 0; i < fig.curves.size(); ++i) {
                if (i < fig.curve_types.size()) {
                    if (fig.curve_types[i] == "2D") 
                        draw_curve(fig, fig.curves[i], fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width, 
                                   fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);
                    else if (fig.curve_types[i] == "HIST") 
                        draw_histogram(fig, fig.curves[i], fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width, 
                                      fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);
                    else if (fig.curve_types[i] == "POLAR") 
                        draw_curve(fig, fig.curves[i], fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width, 
                                   fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);
                }
            }

            // Draw text (title, axes, legend)
            draw_text(fig, fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_width, 
                      fig.is_polar || fig.equal_axes ? std::min(subplot_width, subplot_height) : subplot_height);
        }
    }

    // Restore default view
    texture.setView(texture.getDefaultView());
    texture.display();
}

void PlotGen::draw_axes(const Figure& fig, float w, float h) {
    float margin = 50.0f;
    
    // Draw grids first (to be in the background)
    if (fig.show_major_grid || fig.show_minor_grid) {
        draw_grid(fig, w, h);
    }
    
    // X axis
    sf::VertexArray xAxis(sf::Lines, 2);
    xAxis[0].position = to_screen(fig, fig.xmin, 0, w, h);
    xAxis[1].position = to_screen(fig, fig.xmax, 0, w, h);
    xAxis[0].color = sf::Color::Black;
    xAxis[1].color = sf::Color::Black;
    
    // Y axis
    sf::VertexArray yAxis(sf::Lines, 2);
    yAxis[0].position = to_screen(fig, 0, fig.ymin, w, h);
    yAxis[1].position = to_screen(fig, 0, fig.ymax, w, h);
    yAxis[0].color = sf::Color::Black;
    yAxis[1].color = sf::Color::Black;
    
    texture.draw(xAxis);
    texture.draw(yAxis);
    
    // Ticks on X axis
    const int numTicksX = 5;
    for (int i = 0; i <= numTicksX; ++i) {
        float x = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
        sf::VertexArray tick(sf::Lines, 2);
        tick[0].position = to_screen(fig, x, 0, w, h);
        tick[1].position = to_screen(fig, x, 0, w, h) + sf::Vector2f(0, 5);
        tick[0].color = sf::Color::Black;
        tick[1].color = sf::Color::Black;
        texture.draw(tick);
        
        // Tick value
        sf::Text tickLabel;
        tickLabel.setFont(font);
        tickLabel.setCharacterSize(14);
        tickLabel.setFillColor(sf::Color::Black);
        
        // Use appropriate decimal precision
        std::string tickText;
        if (std::abs(x) < 0.01) {
            tickText = "0";
        } else if (std::abs(x) < 10) {
            char buffer[10];
            std::snprintf(buffer, sizeof(buffer), "%.1f", x);
            tickText = buffer;
        } else {
            tickText = std::to_string(static_cast<int>(x));
        }
        
        tickLabel.setString(sf::String::fromUtf8(tickText.begin(), tickText.end()));
        tickLabel.setPosition(to_screen(fig, x, 0, w, h) + sf::Vector2f(-10, 8));
        texture.draw(tickLabel);
    }
    
    // Ticks on Y axis
    const int numTicksY = 5;
    for (int i = 0; i <= numTicksY; ++i) {
        float y = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
        sf::VertexArray tick(sf::Lines, 2);
        tick[0].position = to_screen(fig, 0, y, w, h);
        tick[1].position = to_screen(fig, 0, y, w, h) + sf::Vector2f(-5, 0);
        tick[0].color = sf::Color::Black;
        tick[1].color = sf::Color::Black;
        texture.draw(tick);
        
        // Tick value
        sf::Text tickLabel;
        tickLabel.setFont(font);
        tickLabel.setCharacterSize(14);
        tickLabel.setFillColor(sf::Color::Black);
        
        // Use appropriate decimal precision
        std::string tickText;
        if (std::abs(y) < 0.01) {
            tickText = "0";
        } else if (std::abs(y) < 10) {
            char buffer[10];
            std::snprintf(buffer, sizeof(buffer), "%.1f", y);
            tickText = buffer;
        } else {
            tickText = std::to_string(static_cast<int>(y));
        }
        
        tickLabel.setString(sf::String::fromUtf8(tickText.begin(), tickText.end()));
        tickLabel.setPosition(to_screen(fig, 0, y, w, h) + sf::Vector2f(-30, -10));
        texture.draw(tickLabel);
    }
}

void PlotGen::draw_grid(const Figure& fig, float w, float h) {
    if (fig.is_polar) {
        draw_polar_grid(fig, w, h);
        return;
    }
    
    // Number of major divisions
    const int numTicksX = 5;
    const int numTicksY = 5;
    
    // Number of minor subdivisions (between each major line)
    const int numMinorSubdivisions = 4;
    
    // Draw major grid
    if (fig.show_major_grid) {
        sf::VertexArray majorGrid(sf::Lines);
        
        // Major vertical lines
        for (int i = 0; i <= numTicksX; ++i) {
            float x = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
            sf::Vector2f top = to_screen(fig, x, fig.ymin, w, h);
            sf::Vector2f bottom = to_screen(fig, x, fig.ymax, w, h);
            
            majorGrid.append(sf::Vertex(top, fig.major_grid_color));
            majorGrid.append(sf::Vertex(bottom, fig.major_grid_color));
        }
        
        // Major horizontal lines
        for (int i = 0; i <= numTicksY; ++i) {
            float y = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
            sf::Vector2f left = to_screen(fig, fig.xmin, y, w, h);
            sf::Vector2f right = to_screen(fig, fig.xmax, y, w, h);
            
            majorGrid.append(sf::Vertex(left, fig.major_grid_color));
            majorGrid.append(sf::Vertex(right, fig.major_grid_color));
        }
        
        texture.draw(majorGrid);
    }
    
    // Draw minor grid
    if (fig.show_minor_grid) {
        sf::VertexArray minorGrid(sf::Lines);
        
        // Minor vertical lines
        for (int i = 0; i < numTicksX; ++i) {
            float x_start = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
            float x_step = (fig.xmax - fig.xmin) / (numTicksX * numMinorSubdivisions);
            
            for (int j = 1; j < numMinorSubdivisions; ++j) {
                float x = x_start + j * x_step;
                sf::Vector2f top = to_screen(fig, x, fig.ymin, w, h);
                sf::Vector2f bottom = to_screen(fig, x, fig.ymax, w, h);
                
                minorGrid.append(sf::Vertex(top, fig.minor_grid_color));
                minorGrid.append(sf::Vertex(bottom, fig.minor_grid_color));
            }
        }
        
        // Minor horizontal lines
        for (int i = 0; i < numTicksY; ++i) {
            float y_start = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
            float y_step = (fig.ymax - fig.ymin) / (numTicksY * numMinorSubdivisions);
            
            for (int j = 1; j < numMinorSubdivisions; ++j) {
                float y = y_start + j * y_step;
                sf::Vector2f left = to_screen(fig, fig.xmin, y, w, h);
                sf::Vector2f right = to_screen(fig, fig.xmax, y, w, h);
                
                minorGrid.append(sf::Vertex(left, fig.minor_grid_color));
                minorGrid.append(sf::Vertex(right, fig.minor_grid_color));
            }
        }
        
        texture.draw(minorGrid);
    }
}

void PlotGen::draw_polar_grid(const Figure& fig, float w, float h) {
    // Center of the polar grid
    sf::Vector2f center = to_screen(fig, 0, 0, w, h);
    
    // Maximum radius for circles (in pixels)
    float max_radius = std::min(w, h) / 2 - 50;
    
    // Maximum radius of data (in units)
    float max_r = std::max(std::abs(fig.xmax), std::abs(fig.ymax));
    
    // Number of concentric circles for major grid
    const int numCircles = 5;
    
    // Number of rays for major grid
    const int numRays = 12; // One ray every 30 degrees
    
    // Draw concentric circles (major grid)
    if (fig.show_major_grid) {
        // Concentric circles
        for (int i = 1; i <= numCircles; ++i) {
            float radius = max_radius * i / numCircles;
            float r_value = max_r * i / numCircles;
            
            sf::CircleShape circle(radius);
            circle.setOrigin(radius, radius);
            circle.setPosition(center);
            circle.setFillColor(sf::Color::Transparent);
            circle.setOutlineColor(fig.major_grid_color);
            circle.setOutlineThickness(1.0f);
            texture.draw(circle);
            
            // Add radius labels
            sf::Text rLabel;
            rLabel.setFont(font);
            rLabel.setCharacterSize(10);
            rLabel.setFillColor(sf::Color::Black);
            rLabel.setString(std::to_string(static_cast<int>(r_value)));
            sf::FloatRect textRect = rLabel.getLocalBounds();
            rLabel.setPosition(center.x + radius * std::cos(3.14f/4) - textRect.width/2, 
                              center.y - radius * std::sin(3.14f/4) - textRect.height/2);
            texture.draw(rLabel);
        }
        
        // Rays
        sf::VertexArray rays(sf::Lines);
        for (int i = 0; i < numRays; ++i) {
            float angle = 2 * 3.14159f * i / numRays;
            sf::Vector2f end(center.x + max_radius * std::cos(angle), 
                            center.y - max_radius * std::sin(angle));
            
            rays.append(sf::Vertex(center, fig.major_grid_color));
            rays.append(sf::Vertex(end, fig.major_grid_color));
            
            // Add angle labels (in degrees)
            sf::Text angleLabel;
            angleLabel.setFont(font);
            angleLabel.setCharacterSize(12);
            angleLabel.setFillColor(sf::Color::Black);
            int degrees = static_cast<int>(angle * 180 / 3.14159f);
            std::string angleText = std::to_string(degrees) + degree_symbol;
            angleLabel.setString(sf::String::fromUtf8(angleText.begin(), angleText.end()));
            sf::FloatRect textRect = angleLabel.getLocalBounds();
            
            // Position the label slightly beyond the end of the ray
            sf::Vector2f labelPos(center.x + (max_radius + 10) * std::cos(angle) - textRect.width/2, 
                                 center.y - (max_radius + 10) * std::sin(angle) - textRect.height/2);
            angleLabel.setPosition(labelPos);
            texture.draw(angleLabel);
        }
        texture.draw(rays);
    }
    
    // Draw additional concentric circles and rays (minor grid)
    if (fig.show_minor_grid) {
        // Minor concentric circles
        const int numMinorSubdivisions = 4;
        for (int i = 1; i < numCircles; ++i) {
            float base_radius = max_radius * i / numCircles;
            float radius_step = max_radius / (numCircles * numMinorSubdivisions);
            
            for (int j = 1; j < numMinorSubdivisions; ++j) {
                float radius = base_radius + j * radius_step;
                
                sf::CircleShape circle(radius);
                circle.setOrigin(radius, radius);
                circle.setPosition(center);
                circle.setFillColor(sf::Color::Transparent);
                circle.setOutlineColor(fig.minor_grid_color);
                circle.setOutlineThickness(1.0f);
                texture.draw(circle);
            }
        }
        
        // Minor rays
        sf::VertexArray minorRays(sf::Lines);
        const int numMinorRays = numRays * 2; // One ray every 15 degrees
        
        for (int i = 0; i < numMinorRays; ++i) {
            // Skip rays that are already in the major grid
            if (i % 2 == 0) continue;
            
            float angle = 2 * 3.14159f * i / numMinorRays;
            sf::Vector2f end(center.x + max_radius * std::cos(angle), 
                            center.y - max_radius * std::sin(angle));
            
            minorRays.append(sf::Vertex(center, fig.minor_grid_color));
            minorRays.append(sf::Vertex(end, fig.minor_grid_color));
        }
        texture.draw(minorRays);
    }
}

void PlotGen::draw_curve(const Figure& fig, const Figure::Curve& curve, float w, float h) {
    if (curve.x.empty() || curve.y.empty()) return;
    
    sf::VertexArray line;
    if (curve.style.line_style == "solid") {
        line.setPrimitiveType(sf::LineStrip);
        for (size_t i = 0; i < curve.x.size(); ++i) {
            sf::Vertex point(to_screen(fig, curve.x[i], curve.y[i], w, h), curve.style.color);
            line.append(point);
            draw_symbol(point.position, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
        }
    } else if (curve.style.line_style == "dashed") {
        line.setPrimitiveType(sf::Lines);
        for (size_t i = 0; i < curve.x.size() - 1; i += 2) {
            sf::Vertex p1(to_screen(fig, curve.x[i], curve.y[i], w, h), curve.style.color);
            sf::Vertex p2(to_screen(fig, curve.x[i + 1], curve.y[i + 1], w, h), curve.style.color);
            line.append(p1);
            line.append(p2);
            draw_symbol(p1.position, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
            draw_symbol(p2.position, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
        }
    } else { // points
        line.setPrimitiveType(sf::Points);
        for (size_t i = 0; i < curve.x.size(); ++i) {
            sf::Vertex point(to_screen(fig, curve.x[i], curve.y[i], w, h), curve.style.color);
            line.append(point);
            draw_symbol(point.position, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
        }
    }
    
    texture.draw(line);
}

void PlotGen::draw_histogram(const Figure& fig, const Figure::Curve& curve, float w, float h) {
    if (curve.x.empty() || curve.y.empty()) return;
    
    float maxY = *std::max_element(curve.y.begin(), curve.y.end());
    if (maxY <= 0) maxY = 1.0f; // Avoid division by zero
    
    // Calculate bin width based on data
    float bin_width = 0;
    if (curve.x.size() > 1) {
        bin_width = curve.x[1] - curve.x[0];
    } else {
        bin_width = (fig.xmax - fig.xmin) / 20.0f;
    }
    
    // Calculate bar width to properly fill the space
    float bar_width = bin_width * w / (fig.xmax - fig.xmin);
    
    // Ensure bars aren't too wide
    bar_width *= curve.bar_width_ratio; // Use custom width ratio
        
    for (size_t i = 0; i < curve.x.size(); ++i) {
        sf::RectangleShape bar;
        float bar_height = curve.y[i] * (h - 100) / (fig.ymax - fig.ymin); // Adjust for margins
        
        // Correct bar position, centered on x value
        sf::Vector2f position = to_screen(fig, curve.x[i], 0, w, h);
        position.x -= bar_width / 2; // Center bar on x value
        
        bar.setSize(sf::Vector2f(bar_width, -bar_height)); // negative to draw upward
        bar.setPosition(position);
        bar.setFillColor(curve.style.color);
        bar.setOutlineColor(sf::Color::Black);
        bar.setOutlineThickness(1.0f);
        
        texture.draw(bar);
    }
}

sf::Color PlotGen::getColorFromHeight(float height) {
    // Create a color gradient blue->cyan->green->yellow->red
    if (height < 0.25f) {
        // Blue to cyan
        return sf::Color(0, static_cast<sf::Uint8>(255 * height * 4), 255);
    } else if (height < 0.5f) {
        // Cyan to green
        return sf::Color(0, 255, static_cast<sf::Uint8>(255 * (1 - (height - 0.25f) * 4)));
    } else if (height < 0.75f) {
        // Green to yellow
        return sf::Color(static_cast<sf::Uint8>(255 * (height - 0.5f) * 4), 255, 0);
    } else {
        // Yellow to red
        return sf::Color(255, static_cast<sf::Uint8>(255 * (1 - (height - 0.75f) * 4)), 0);
    }
}

sf::Vector2f PlotGen::to_screen(const Figure& fig, float x, float y, float w, float h) const {
    float margin = 50.0f;
    
    // Calculate screen coordinates
    float sx = margin + (x - fig.xmin) / (fig.xmax - fig.xmin) * (w - 2 * margin);
    float sy = h - margin - (y - fig.ymin) / (fig.ymax - fig.ymin) * (h - 2 * margin);
    
    // Ensure points stay within graph boundaries
    sx = std::max(margin, std::min(w - margin, sx));
    sy = std::max(margin, std::min(h - margin, sy));
    
    return {sx, sy};
}

void PlotGen::draw_text(const Figure& fig, float w, float h) {
    sf::Text text;
    text.setFont(font);
    text.setFillColor(sf::Color::Black);

    // Title with larger font size for better readability
    text.setCharacterSize(24);
    text.setString(sf::String::fromUtf8(fig.title.begin(), fig.title.end()));
    sf::FloatRect textRect = text.getLocalBounds();
    text.setPosition(w / 2 - textRect.width / 2, 10);
    texture.draw(text);

    // X label
    text.setCharacterSize(18);
    text.setString(sf::String::fromUtf8(fig.xlabel.begin(), fig.xlabel.end()));
    textRect = text.getLocalBounds();
    text.setPosition(w / 2 - textRect.width / 2, h - 25);
    texture.draw(text);

    // Y label
    text.setCharacterSize(18);
    text.setString(sf::String::fromUtf8(fig.ylabel.begin(), fig.ylabel.end()));
    textRect = text.getLocalBounds();
    text.setRotation(-90);
    text.setPosition(15, h / 2 + textRect.width / 2);
    texture.draw(text);
    text.setRotation(0); // Reset to 0 for subsequent text

    // Legend
    if (fig.show_leg) {
        float legend_x = w - 180;
        float legend_y = 50;
        const float max_legend_width = 130.0f;  // Maximum legend width in pixels
        
        // Semi-transparent legend background
        if (!fig.curves.empty()) {
            int total_legend_height = 0;
            std::vector<std::pair<sf::Color, std::vector<std::string>>> legend_items;
            
            // Prepare and cut too long legends
            for (const auto& curve : fig.curves) {
                if (!curve.style.legend.empty()) {
                    std::vector<std::string> legend_lines;
                    
                    // Cut legend into multiple lines if necessary
                    std::string legend_text = curve.style.legend;
                    text.setCharacterSize(16);
                    text.setString(sf::String::fromUtf8(legend_text.begin(), legend_text.end()));
                    sf::FloatRect bounds = text.getLocalBounds();
                    
                    if (bounds.width > max_legend_width) {
                        // Cut legend into words
                        std::vector<std::string> words;
                        std::string current_word;
                        for (char c : legend_text) {
                            if (c == ' ' || c == '\t' || c == '\n') {
                                if (!current_word.empty()) {
                                    words.push_back(current_word);
                                    current_word.clear();
                                }
                                if (c == '\n') {
                                    words.push_back("\n");
                                }
                            } else {
                                current_word += c;
                            }
                        }
                        if (!current_word.empty()) {
                            words.push_back(current_word);
                        }
                        
                        // Build lines
                        std::string current_line;
                        for (const std::string& word : words) {
                            if (word == "\n") {
                                if (!current_line.empty()) {
                                    legend_lines.push_back(current_line);
                                    current_line.clear();
                                }
                                continue;
                            }
                            
                            std::string test_line = current_line;
                            if (!test_line.empty()) test_line += " ";
                            test_line += word;
                            
                            text.setString(sf::String::fromUtf8(test_line.begin(), test_line.end()));
                            bounds = text.getLocalBounds();
                            
                            if (bounds.width <= max_legend_width) {
                                current_line = test_line;
                            } else {
                                if (!current_line.empty()) {
                                    legend_lines.push_back(current_line);
                                }
                                current_line = word;
                            }
                        }
                        
                        if (!current_line.empty()) {
                            legend_lines.push_back(current_line);
                        }
                    } else {
                        legend_lines.push_back(legend_text);
                    }
                    
                    // Add this entry
                    legend_items.push_back({curve.style.color, legend_lines});
                    
                    // Calculate total height needed
                    total_legend_height += 10 + legend_lines.size() * 20; // 10px padding, 20px per line
                }
            }
            
            if (!legend_items.empty()) {
                // Draw legend background
                sf::RectangleShape legendBg;
                legendBg.setSize(sf::Vector2f(160, 20 + total_legend_height));
                legendBg.setPosition(legend_x - 10, legend_y - 10);
                legendBg.setFillColor(sf::Color(255, 255, 255, 220));
                legendBg.setOutlineColor(sf::Color::Black);
                legendBg.setOutlineThickness(1.0f);
                texture.draw(legendBg);
                
                // Draw each legend item
                text.setCharacterSize(16);
                for (const auto& item : legend_items) {
                    // Thicker legend line/symbol
                    sf::RectangleShape legendLine;
                    legendLine.setSize(sf::Vector2f(30, 3));
                    legendLine.setPosition(legend_x, legend_y + 7);
                    legendLine.setFillColor(item.first);
                    texture.draw(legendLine);
                    
                    // Legend text with multi-line support
                    float line_y = legend_y;
                    for (const auto& line : item.second) {
                        text.setString(sf::String::fromUtf8(line.begin(), line.end()));
                        text.setPosition(legend_x + 40, line_y);
                        texture.draw(text);
                        line_y += 20;
                    }
                    
                    // Move to next item
                    legend_y += 10 + item.second.size() * 20;
                }
            }
        }
    }
}

// Method for drawing a symbol at a given position
void PlotGen::draw_symbol(const sf::Vector2f& position, const std::string& symbol_type, float size, const sf::Color& color) {
    if (symbol_type == "none") return;
    
    if (symbol_type == "circle") {
        sf::CircleShape circle(size / 2);
        circle.setOrigin(size / 2, size / 2);
        circle.setPosition(position);
        circle.setFillColor(color);
        circle.setOutlineColor(sf::Color::Black);
        circle.setOutlineThickness(1.0f);
        texture.draw(circle);
    } 
    else if (symbol_type == "square") {
        sf::RectangleShape square(sf::Vector2f(size, size));
        square.setOrigin(size / 2, size / 2);
        square.setPosition(position);
        square.setFillColor(color);
        square.setOutlineColor(sf::Color::Black);
        square.setOutlineThickness(1.0f);
        texture.draw(square);
    }
    else if (symbol_type == "triangle") {
        sf::CircleShape triangle(size / 2, 3); // triangle = circle with 3 sides
        triangle.setOrigin(size / 2, size / 2);
        triangle.setPosition(position);
        triangle.setFillColor(color);
        triangle.setOutlineColor(sf::Color::Black);
        triangle.setOutlineThickness(1.0f);
        texture.draw(triangle);
    }
    else if (symbol_type == "diamond") {
        sf::CircleShape diamond(size / 2, 4); // diamond = circle with 4 sides
        diamond.setOrigin(size / 2, size / 2);
        diamond.setPosition(position);
        diamond.setRotation(45.0f); // 45° rotation to get a diamond
        diamond.setFillColor(color);
        diamond.setOutlineColor(sf::Color::Black);
        diamond.setOutlineThickness(1.0f);
        texture.draw(diamond);
    }
    else if (symbol_type == "star") {
        // Create a 5-pointed star
        const int numPoints = 5;
        const float innerRadius = size / 4;
        const float outerRadius = size / 2;
        
        sf::ConvexShape star;
        star.setPointCount(numPoints * 2);
        star.setOrigin(size / 2, size / 2);
        star.setPosition(position);
        star.setFillColor(color);
        star.setOutlineColor(sf::Color::Black);
        star.setOutlineThickness(1.0f);
        
        for (int i = 0; i < numPoints * 2; i++) {
            float radius = (i % 2 == 0) ? outerRadius : innerRadius;
            float angle = i * 3.14159f / numPoints;
            sf::Vector2f point(radius * std::cos(angle), radius * std::sin(angle));
            star.setPoint(i, point);
        }
        
        texture.draw(star);
    }
}