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
    double thickness_,
    const std::string &line_style_,
    const std::string &legend_,
    const std::string &symbol_type_,
    double symbol_size_) : color(color_), thickness(thickness_), line_style(line_style_),
                           legend(legend_), symbol_type(symbol_type_), symbol_size(symbol_size_)
{
}

// Constructor
PlotGen::PlotGen(unsigned int width, unsigned int height, unsigned int rows, unsigned int cols)
    : width(width), height(height), rows(rows), cols(cols) // Ne pas initialiser la fenêtre ici
{
    // Créer uniquement la texture pour le rendu, mais pas la fenêtre visible
    texture.create(width, height);
    texture.setSmooth(true);

    // Search for the font in several possible locations
    if (font.loadFromFile("fonts/arial.ttf"))
    {
        std::cout << "Font loaded from current directory" << std::endl;
    }
    else if (font.loadFromFile("build/arial.ttf"))
    {
        std::cout << "Font loaded from build directory" << std::endl;
    }
    else if (font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"))
    {
        std::cout << "LiberationSans font loaded" << std::endl;
    }
    else if (font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
    {
        std::cout << "DejaVuSans font loaded" << std::endl;
    }
    else
    {
        std::cerr << "WARNING: Unable to load a font compatible with Unicode characters" << std::endl;
        throw std::runtime_error("Unable to load a font supporting Unicode");
    }

    figures.resize(rows * cols);
    for (auto &fig : figures)
    {
        fig.curve_types.clear();
        fig.curves.clear();
    }
}

// Add a figure at a position (row, col)
PlotGen::Figure &PlotGen::subplot(unsigned int row, unsigned int col)
{
    if (row >= rows || col >= cols)
    {
        throw std::out_of_range("Subplot index out of range");
    }
    return figures[row * cols + col];
}

// Figure configuration
void PlotGen::set_title(Figure &fig, const std::string &title) { fig.title = title; }
void PlotGen::set_xlabel(Figure &fig, const std::string &label) { fig.xlabel = label; }
void PlotGen::set_ylabel(Figure &fig, const std::string &label) { fig.ylabel = label; }
void PlotGen::set_axis_limits(Figure &fig, double xmin, double xmax, double ymin, double ymax)
{
    fig.xmin = xmin;
    fig.xmax = xmax;
    fig.ymin = ymin;
    fig.ymax = ymax;
}

// New function specific for polar graphs ensuring axes of the same dimension
void PlotGen::set_polar_axis_limits(Figure &fig, double max_radius)
{
    // For a polar graph, X and Y axes must have the same scale
    // and be centered on (0,0)
    fig.is_polar = true; // Mark as a polar graph
    fig.xmin = -max_radius;
    fig.xmax = max_radius;
    fig.ymin = -max_radius;
    fig.ymax = max_radius;
}

void PlotGen::show_legend(Figure &fig, bool show) { fig.show_leg = show; }

// Nouvelle méthode pour définir la position de la légende
void PlotGen::set_legend_position(Figure &fig, const std::string &position) { 
    // Vérifier que la position est valide
    if (position == "top-right" || position == "top-left" || 
        position == "bottom-right" || position == "bottom-left" || 
        position == "outside-right") {
        fig.legend_position = position;
    } else {
        std::cerr << "WARNING: Invalid legend position. Using default 'top-right'." << std::endl;
        fig.legend_position = "top-right";
    }
}

// Methods to enable/disable grids
void PlotGen::grid(Figure &fig, bool major, bool minor)
{
    fig.show_major_grid = major;
    fig.show_minor_grid = minor;
}

void PlotGen::set_grid_color(Figure &fig, sf::Color major_color, sf::Color minor_color)
{
    fig.major_grid_color = major_color;
    fig.minor_grid_color = minor_color;
}

void PlotGen::set_equal_axes(Figure &fig, bool equal) { fig.equal_axes = equal; }

// 2D curve plotting
void PlotGen::plot(Figure& fig, const std::vector<double>& x, const std::vector<double>& y, const Style& style)
{
    if (x.size() != y.size() || x.empty())
    {
        throw std::invalid_argument("x and y vectors must have the same size and not be empty");
    }

    // Ajouter un auto-ajustement des limites d'axes si elles n'ont pas été définies
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Trouver les min/max pour x et y
        double x_min = *std::min_element(x.begin(), x.end());
        double x_max = *std::max_element(x.begin(), x.end());
        double y_min = *std::min_element(y.begin(), y.end());
        double y_max = *std::max_element(y.begin(), y.end());

        // Ajouter une marge de 5% pour une meilleure visualisation
        double x_margin = (x_max - x_min) * 0.05;
        double y_margin = (y_max - y_min) * 0.05;

        // Cas particulier pour éviter des problèmes si toutes les valeurs sont égales
        if (std::abs(x_max - x_min) < 1e-10)
        {
            x_margin = 1.0;
        }
        if (std::abs(y_max - y_min) < 1e-10)
        {
            y_margin = 1.0;
        }

        // Définir les nouvelles limites
        fig.xmin = x_min - x_margin;
        fig.xmax = x_max + x_margin;
        fig.ymin = y_min - y_margin;
        fig.ymax = y_max + y_margin;
    }

    // Utiliser directement le style sans ajouter de description textuelle
    fig.curves.push_back({x, y, style});
    fig.curve_types.push_back("2D");
}

// Circle with center (x0, y0) and radius r
void PlotGen::circle(Figure& fig, double x0, double y0, double r, const Style& style)
{
    // Validation du rayon
    if (r <= 0)
    {
        throw std::invalid_argument("Circle radius must be positive");
    }

    // Générer les points du cercle en utilisant les équations paramétriques
    const int num_points = 100; // Nombre de points pour dessiner le cercle
    std::vector<double> x(num_points), y(num_points);

    for (int i = 0; i < num_points; ++i)
    {
        double angle = 2.0 * M_PI * i / (num_points - 1);
        x[i] = x0 + r * std::cos(angle);
        y[i] = y0 + r * std::sin(angle);
    }

    // Activer l'option equal_axes pour garantir que le cercle est bien rond
    bool was_equal = fig.equal_axes;
    fig.equal_axes = true;

    // Vérifier si les limites d'axes existantes sont suffisantes
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);
    
    if (using_default_limits)
    {
        // Définir de nouvelles limites d'axes pour bien voir le cercle avec une marge
        double margin = r * 0.2; // 20% de marge autour du cercle
        fig.xmin = x0 - r - margin;
        fig.xmax = x0 + r + margin;
        fig.ymin = y0 - r - margin;
        fig.ymax = y0 + r + margin;
    }
    else
    {
        // Vérifier si le cercle est visible dans les limites actuelles
        double min_x = x0 - r;
        double max_x = x0 + r;
        double min_y = y0 - r;
        double max_y = y0 + r;
        
        // Étendre les limites si nécessaire
        if (min_x < fig.xmin) fig.xmin = min_x - r * 0.1;
        if (max_x > fig.xmax) fig.xmax = max_x + r * 0.1;
        if (min_y < fig.ymin) fig.ymin = min_y - r * 0.1;
        if (max_y > fig.ymax) fig.ymax = max_y + r * 0.1;
    }

    // Tracer le cercle comme une courbe normale
    plot(fig, x, y, style);

    // Restaurer l'état précédent de equal_axes si nécessaire
    fig.equal_axes = was_equal;
}

// Histogram
void PlotGen::hist(Figure &fig, const std::vector<double> &data, int bins, const Style &style, double bar_width_ratio)
{
    if (data.empty())
    {
        throw std::invalid_argument("data vector must not be empty");
    }

    std::vector<double> hist_x, hist_y;
    double min_val = *std::min_element(data.begin(), data.end());
    double max_val = *std::max_element(data.begin(), data.end());
    double bin_width = (max_val - min_val) / bins;

    std::vector<int> counts(bins, 0);
    for (double val : data)
    {
        int bin = std::min(static_cast<int>((val - min_val) / bin_width), bins - 1);
        counts[bin]++;
    }

    for (int i = 0; i < bins; ++i)
    {
        hist_x.push_back(min_val + i * bin_width);
        hist_y.push_back(static_cast<double>(counts[i]));
    }

    // Automatically set axis limits if they haven't been specified
    // Check if default limits are being used
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    if (using_default_limits)
    {
        // Set X limits to include all bars with a 5% margin
        double x_margin = (max_val - min_val) * 0.05f;
        fig.xmin = min_val - x_margin;
        fig.xmax = max_val + bin_width + x_margin; // Add bin_width to see the last bar completely

        // Set Y limits to include all bars with a 10% margin
        double max_count = *std::max_element(hist_y.begin(), hist_y.end());
        double y_margin = max_count * 0.1f;
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
void PlotGen::polar_plot(Figure &fig, const std::vector<double> &theta, const std::vector<double> &r, const Style &style)
{
    if (theta.size() != r.size() || theta.empty())
    {
        throw std::invalid_argument("theta and r vectors must have the same size and not be empty");
    }

    // Mark the figure as being in polar coordinates
    fig.is_polar = true;

    // Find the maximum radius to adapt the graph limits
    double max_r = *std::max_element(r.begin(), r.end());

    // Check if we're using default limits
    bool using_default_limits = (fig.xmin == -10 && fig.xmax == 10 && fig.ymin == -10 && fig.ymax == 10);

    // If the limits haven't been explicitly set or are default, calculate them automatically
    if (using_default_limits)
    {
        // Add a 10% margin for better visualization
        set_polar_axis_limits(fig, max_r * 1.1f);
    }
    else
    {
        // Check if current limits are sufficient to contain all data
        // and if they respect polar graph constraints (axes of same dimension)
        double current_range_x = (fig.xmax - fig.xmin) / 2;
        double current_range_y = (fig.ymax - fig.ymin) / 2;
        double needed_range = max_r * 1.1f;

        // Ensure axes have the same scale
        double max_range = std::max(current_range_x, current_range_y);

        if (needed_range > max_range || std::abs(current_range_x - current_range_y) > 1e-6)
        {
            // Use set_polar_axis_limits to ensure axes of same dimension
            double max_limit = std::max(needed_range, max_range);
            set_polar_axis_limits(fig, max_limit);
        }
    }

    // Convert to Cartesian coordinates for display
    std::vector<double> x, y;
    for (size_t i = 0; i < theta.size(); ++i)
    {
        x.push_back(r[i] * std::cos(theta[i]));
        y.push_back(r[i] * std::sin(theta[i]));
    }

    fig.curves.push_back({x, y, style});
    fig.curve_types.push_back("POLAR");
}

// Display and render
void PlotGen::show()
{
    // Effectuer le rendu
    render();
    
    // Créer la fenêtre seulement lorsque show() est explicitement appelé
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    window.create(sf::VideoMode(width, height), "PlotGen", sf::Style::Default, settings);
    
    // Configurer le sprite
    sprite.setTexture(texture.getTexture());

    // Main loop
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }
        }

        // Render and display
        window.clear(sf::Color::White);
        window.draw(sprite);
        window.display();
    }
}

// Save
void PlotGen::save(const std::string &filename)
{
    render(); // Ensure the rendering is up to date

    sf::Image screenshot = texture.getTexture().copyToImage();

    if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".png") == 0)
    {
        if (!screenshot.saveToFile(filename))
        {
            throw std::runtime_error("Unable to save image in PNG format");
        }
    }
    else if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".jpg") == 0)
    {
        // Conversion for stb_image_write with high quality
        std::vector<unsigned char> pixels(width * height * 3); // RGB
        for (unsigned int y = 0; y < height; ++y)
        {
            for (unsigned int x = 0; x < width; ++x)
            {
                sf::Color c = screenshot.getPixel(x, y);
                size_t index = (y * width + x) * 3;
                pixels[index] = c.r;
                pixels[index + 1] = c.g;
                pixels[index + 2] = c.b;
            }
        }

        // Write JPG image with 95% quality (better quality)
        if (!stbi_write_jpg(filename.c_str(), width, height, 3, pixels.data(), 95))
        {
            throw std::runtime_error("Unable to save image in JPG format");
        }
    }
    else
    {
        throw std::invalid_argument("File format not supported. Use .png or .jpg");
    }

    std::cout << "Image saved to: " << filename << " (Dimensions: " << width << "x" << height << ")" << std::endl;
}

void PlotGen::render()
{
    texture.clear(sf::Color::White);

    // Go through all subplots
    for (unsigned int row = 0; row < rows; ++row)
    {
        for (unsigned int col = 0; col < cols; ++col)
        {
            Figure &fig = figures[row * cols + col];

            // Define the view for this subplot
            double subplot_width = static_cast<double>(width) / cols;
            double subplot_height = static_cast<double>(height) / rows;

            // For polar graphs, ensure dimensions are square
            if (fig.is_polar)
            {
                double min_size = std::min(subplot_width, subplot_height);
                double x_offset = (subplot_width - min_size) / 2.0f;
                double y_offset = (subplot_height - min_size) / 2.0f;

                sf::View view(sf::FloatRect(0, 0, min_size, min_size));
                view.setViewport(sf::FloatRect(
                    static_cast<double>(col) / cols + x_offset / width,
                    static_cast<double>(row) / rows + y_offset / height,
                    min_size / width,
                    min_size / height));
                texture.setView(view);
            }
            else if (fig.equal_axes)
            {
                double min_size = std::min(subplot_width, subplot_height);
                double x_offset = (subplot_width - min_size) / 2.0f;
                double y_offset = (subplot_height - min_size) / 2.0f;

                sf::View view(sf::FloatRect(0, 0, min_size, min_size));
                view.setViewport(sf::FloatRect(
                    static_cast<double>(col) / cols + x_offset / width,
                    static_cast<double>(row) / rows + y_offset / height,
                    min_size / width,
                    min_size / height));
                texture.setView(view);
            }
            else
            {
                sf::View view(sf::FloatRect(0, 0, subplot_width, subplot_height));
                view.setViewport(sf::FloatRect(
                    static_cast<double>(col) / cols,
                    static_cast<double>(row) / rows,
                    1.0f / cols,
                    1.0f / rows));
                texture.setView(view);
            }

            // Draw the subplot frame
            sf::RectangleShape frame;
            if (fig.is_polar || fig.equal_axes)
            {
                double min_size = std::min(subplot_width, subplot_height);
                frame.setSize(sf::Vector2f(min_size, min_size));
            }
            else
            {
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
            for (size_t i = 0; i < fig.curves.size(); ++i)
            {
                if (i < fig.curve_types.size())
                {
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

void PlotGen::draw_axes(const Figure &fig, double w, double h)
{
    // double margin = 50.0f;

    // Draw grids first (to be in the background)
    if (fig.show_major_grid || fig.show_minor_grid)
    {
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
    for (int i = 0; i <= numTicksX; ++i)
    {
        double x = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
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
        if (std::abs(x) < 0.01)
        {
            tickText = "0";
        }
        else if (std::abs(x) < 10)
        {
            char buffer[10];
            std::snprintf(buffer, sizeof(buffer), "%.1f", x);
            tickText = buffer;
        }
        else
        {
            tickText = std::to_string(static_cast<int>(x));
        }

        tickLabel.setString(sf::String::fromUtf8(tickText.begin(), tickText.end()));
        tickLabel.setPosition(to_screen(fig, x, 0, w, h) + sf::Vector2f(-10, 8));
        texture.draw(tickLabel);
    }

    // Ticks on Y axis
    const int numTicksY = 5;
    for (int i = 0; i <= numTicksY; ++i)
    {
        double y = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
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
        if (std::abs(y) < 0.01)
        {
            tickText = "0";
        }
        else if (std::abs(y) < 10)
        {
            char buffer[10];
            std::snprintf(buffer, sizeof(buffer), "%.1f", y);
            tickText = buffer;
        }
        else
        {
            tickText = std::to_string(static_cast<int>(y));
        }

        tickLabel.setString(sf::String::fromUtf8(tickText.begin(), tickText.end()));
        tickLabel.setPosition(to_screen(fig, 0, y, w, h) + sf::Vector2f(-30, -10));
        texture.draw(tickLabel);
    }
}

void PlotGen::draw_grid(const Figure &fig, double w, double h)
{
    if (fig.is_polar)
    {
        draw_polar_grid(fig, w, h);
        return;
    }

    // Number of major divisions
    const int numTicksX = 5;
    const int numTicksY = 5;

    // Number of minor subdivisions (between each major line)
    const int numMinorSubdivisions = 4;

    // Draw major grid
    if (fig.show_major_grid)
    {
        sf::VertexArray majorGrid(sf::Lines);

        // Major vertical lines
        for (int i = 0; i <= numTicksX; ++i)
        {
            double x = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
            sf::Vector2f top = to_screen(fig, x, fig.ymin, w, h);
            sf::Vector2f bottom = to_screen(fig, x, fig.ymax, w, h);

            majorGrid.append(sf::Vertex(top, fig.major_grid_color));
            majorGrid.append(sf::Vertex(bottom, fig.major_grid_color));
        }

        // Major horizontal lines
        for (int i = 0; i <= numTicksY; ++i)
        {
            double y = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
            sf::Vector2f left = to_screen(fig, fig.xmin, y, w, h);
            sf::Vector2f right = to_screen(fig, fig.xmax, y, w, h);

            majorGrid.append(sf::Vertex(left, fig.major_grid_color));
            majorGrid.append(sf::Vertex(right, fig.major_grid_color));
        }

        texture.draw(majorGrid);
    }

    // Draw minor grid
    if (fig.show_minor_grid)
    {
        sf::VertexArray minorGrid(sf::Lines);

        // Minor vertical lines
        for (int i = 0; i < numTicksX; ++i)
        {
            double x_start = fig.xmin + (fig.xmax - fig.xmin) * i / numTicksX;
            double x_step = (fig.xmax - fig.xmin) / (numTicksX * numMinorSubdivisions);

            for (int j = 1; j < numMinorSubdivisions; ++j)
            {
                double x = x_start + j * x_step;
                sf::Vector2f top = to_screen(fig, x, fig.ymin, w, h);
                sf::Vector2f bottom = to_screen(fig, x, fig.ymax, w, h);

                minorGrid.append(sf::Vertex(top, fig.minor_grid_color));
                minorGrid.append(sf::Vertex(bottom, fig.minor_grid_color));
            }
        }

        // Minor horizontal lines
        for (int i = 0; i < numTicksY; ++i)
        {
            double y_start = fig.ymin + (fig.ymax - fig.ymin) * i / numTicksY;
            double y_step = (fig.ymax - fig.ymin) / (numTicksY * numMinorSubdivisions);

            for (int j = 1; j < numMinorSubdivisions; ++j)
            {
                double y = y_start + j * y_step;
                sf::Vector2f left = to_screen(fig, fig.xmin, y, w, h);
                sf::Vector2f right = to_screen(fig, fig.xmax, y, w, h);

                minorGrid.append(sf::Vertex(left, fig.minor_grid_color));
                minorGrid.append(sf::Vertex(right, fig.minor_grid_color));
            }
        }

        texture.draw(minorGrid);
    }
}

void PlotGen::draw_polar_grid(const Figure &fig, double w, double h)
{
    // Center of the polar grid
    sf::Vector2f center = to_screen(fig, 0, 0, w, h);

    // Maximum radius for circles (in pixels)
    double max_radius = std::min(w, h) / 2 - 50;

    // Maximum radius of data (in units)
    double max_r = std::max(std::abs(fig.xmax), std::abs(fig.ymax));

    // Number of concentric circles for major grid
    const int numCircles = 5;

    // Number of rays for major grid
    const int numRays = 12; // One ray every 30 degrees

    // Draw concentric circles (major grid)
    if (fig.show_major_grid)
    {
        // Concentric circles
        for (int i = 1; i <= numCircles; ++i)
        {
            double radius = max_radius * i / numCircles;
            double r_value = max_r * i / numCircles;

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
            rLabel.setPosition(center.x + radius * std::cos(3.14f / 4) - textRect.width / 2,
                               center.y - radius * std::sin(3.14f / 4) - textRect.height / 2);
            texture.draw(rLabel);
        }

        // Rays
        sf::VertexArray rays(sf::Lines);
        for (int i = 0; i < numRays; ++i)
        {
            double angle = 2 * 3.14159f * i / numRays;
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
            sf::Vector2f labelPos(center.x + (max_radius + 10) * std::cos(angle) - textRect.width / 2,
                                  center.y - (max_radius + 10) * std::sin(angle) - textRect.height / 2);
            angleLabel.setPosition(labelPos);
            texture.draw(angleLabel);
        }
        texture.draw(rays);
    }

    // Draw additional concentric circles and rays (minor grid)
    if (fig.show_minor_grid)
    {
        // Minor concentric circles
        const int numMinorSubdivisions = 4;
        for (int i = 1; i < numCircles; ++i)
        {
            double base_radius = max_radius * i / numCircles;
            double radius_step = max_radius / (numCircles * numMinorSubdivisions);

            for (int j = 1; j < numMinorSubdivisions; ++j)
            {
                double radius = base_radius + j * radius_step;

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

        for (int i = 0; i < numMinorRays; ++i)
        {
            // Skip rays that are already in the major grid
            if (i % 2 == 0)
                continue;

            double angle = 2 * 3.14159f * i / numMinorRays;
            sf::Vector2f end(center.x + max_radius * std::cos(angle),
                             center.y - max_radius * std::sin(angle));

            minorRays.append(sf::Vertex(center, fig.minor_grid_color));
            minorRays.append(sf::Vertex(end, fig.minor_grid_color));
        }
        texture.draw(minorRays);
    }
}

void PlotGen::draw_curve(const Figure &fig, const Figure::Curve &curve, double w, double h)
{
    if (curve.x.empty() || curve.y.empty())
        return;

    // Appliquer l'épaisseur des lignes en utilisant les vertex arrays avec triangles
    // pour les lignes de plus d'un pixel d'épaisseur
    float thickness = static_cast<float>(curve.style.thickness);

    if (curve.style.line_style == "solid")
    {
        if (thickness <= 1.0f)
        {
            // Pour les lignes fines, utiliser LineStrip (plus efficace)
            sf::VertexArray line(sf::LineStrip);
            for (size_t i = 0; i < curve.x.size(); ++i)
            {
                sf::Vertex point(to_screen(fig, curve.x[i], curve.y[i], w, h), curve.style.color);
                line.append(point);
                draw_symbol(point.position, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
            }
            texture.draw(line);
        }
        else
        {
            // Pour les lignes épaisses, utiliser des triangles
            if (curve.x.size() > 1)
            {
                sf::VertexArray thickLine(sf::Triangles);

                for (size_t i = 0; i < curve.x.size() - 1; ++i)
                {
                    sf::Vector2f p1 = to_screen(fig, curve.x[i], curve.y[i], w, h);
                    sf::Vector2f p2 = to_screen(fig, curve.x[i + 1], curve.y[i + 1], w, h);

                    // Calculer le vecteur normalisé perpendiculaire à la ligne
                    sf::Vector2f direction = p2 - p1;
                    sf::Vector2f unitDirection;
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                    if (length > 0)
                    {
                        unitDirection = direction / length;
                        sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

                        // Calculer les points pour former un rectangle (deux triangles)
                        sf::Vector2f offset = unitPerpendicular * (thickness / 2.0f);

                        sf::Vertex v1(p1 + offset, curve.style.color);
                        sf::Vertex v2(p2 + offset, curve.style.color);
                        sf::Vertex v3(p2 - offset, curve.style.color);
                        sf::Vertex v4(p1 - offset, curve.style.color);

                        // Premier triangle
                        thickLine.append(v1);
                        thickLine.append(v2);
                        thickLine.append(v3);

                        // Second triangle
                        thickLine.append(v1);
                        thickLine.append(v3);
                        thickLine.append(v4);
                    }

                    // Dessiner les symboles
                    draw_symbol(p1, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
                }

                // Dessiner le dernier symbole
                if (!curve.x.empty())
                {
                    sf::Vector2f lastPoint = to_screen(fig, curve.x.back(), curve.y.back(), w, h);
                    draw_symbol(lastPoint, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
                }

                texture.draw(thickLine);
            }
        }
    }
    else if (curve.style.line_style == "dashed")
    {
        // Pour les lignes pointillées
        if (thickness <= 1.0f)
        {
            // Lignes fines
            sf::VertexArray line(sf::Lines);
            for (size_t i = 0; i < curve.x.size() - 1; i += 2)
            {
                sf::Vertex p1(to_screen(fig, curve.x[i], curve.y[i], w, h), curve.style.color);
                sf::Vertex p2(to_screen(fig, curve.x[i + 1], curve.y[i + 1], w, h), curve.style.color);
                line.append(p1);
                line.append(p2);
                draw_symbol(p1.position, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
                draw_symbol(p2.position, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
            }
            texture.draw(line);
        }
        else
        {
            // Lignes épaisses pointillées
            if (curve.x.size() > 1)
            {
                sf::VertexArray thickLine(sf::Triangles);

                for (size_t i = 0; i < curve.x.size() - 1; i += 2)
                {
                    sf::Vector2f p1 = to_screen(fig, curve.x[i], curve.y[i], w, h);
                    sf::Vector2f p2 = to_screen(fig, curve.x[i + 1], curve.y[i + 1], w, h);

                    // Même logique que pour les lignes pleines
                    sf::Vector2f direction = p2 - p1;
                    sf::Vector2f unitDirection;
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                    if (length > 0)
                    {
                        unitDirection = direction / length;
                        sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

                        sf::Vector2f offset = unitPerpendicular * (thickness / 2.0f);

                        sf::Vertex v1(p1 + offset, curve.style.color);
                        sf::Vertex v2(p2 + offset, curve.style.color);
                        sf::Vertex v3(p2 - offset, curve.style.color);
                        sf::Vertex v4(p1 - offset, curve.style.color);

                        thickLine.append(v1);
                        thickLine.append(v2);
                        thickLine.append(v3);

                        thickLine.append(v1);
                        thickLine.append(v3);
                        thickLine.append(v4);
                    }

                    draw_symbol(p1, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
                    draw_symbol(p2, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
                }

                texture.draw(thickLine);
            }
        }
    }
    else
    { // points
        sf::VertexArray line(sf::Points);
        for (size_t i = 0; i < curve.x.size(); ++i)
        {
            sf::Vertex point(to_screen(fig, curve.x[i], curve.y[i], w, h), curve.style.color);
            line.append(point);
            draw_symbol(point.position, curve.style.symbol_type, curve.style.symbol_size, curve.style.color);
        }
        texture.draw(line);
    }
}

void PlotGen::draw_histogram(const Figure &fig, const Figure::Curve &curve, double w, double h)
{
    if (curve.x.empty() || curve.y.empty())
        return;

    double maxY = *std::max_element(curve.y.begin(), curve.y.end());
    if (maxY <= 0)
        maxY = 1.0f; // Avoid division by zero

    // Calculate bin width based on data
    double bin_width = 0;
    if (curve.x.size() > 1)
    {
        bin_width = curve.x[1] - curve.x[0];
    }
    else
    {
        bin_width = (fig.xmax - fig.xmin) / 20.0f;
    }

    // Calculate bar width to properly fill the space
    double bar_width = bin_width * w / (fig.xmax - fig.xmin);

    // Ensure bars aren't too wide
    bar_width *= curve.bar_width_ratio; // Use custom width ratio

    for (size_t i = 0; i < curve.x.size(); ++i)
    {
        sf::RectangleShape bar;
        double bar_height = curve.y[i] * (h - 100) / (fig.ymax - fig.ymin); // Adjust for margins

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

sf::Color PlotGen::getColorFromHeight(double height)
{
    // Create a color gradient blue->cyan->green->yellow->red
    if (height < 0.25f)
    {
        // Blue to cyan
        return sf::Color(0, static_cast<sf::Uint8>(255 * height * 4), 255);
    }
    else if (height < 0.5f)
    {
        // Cyan to green
        return sf::Color(0, 255, static_cast<sf::Uint8>(255 * (1 - (height - 0.25f) * 4)));
    }
    else if (height < 0.75f)
    {
        // Green to yellow
        return sf::Color(static_cast<sf::Uint8>(255 * (height - 0.5f) * 4), 255, 0);
    }
    else
    {
        // Yellow to red
        return sf::Color(255, static_cast<sf::Uint8>(255 * (1 - (height - 0.75f) * 4)), 0);
    }
}

sf::Vector2f PlotGen::to_screen(const Figure &fig, double x, double y, double w, double h) const
{
    double margin = 50.0f;

    // Calculate screen coordinates
    double sx = margin + (x - fig.xmin) / (fig.xmax - fig.xmin) * (w - 2 * margin);
    double sy = h - margin - (y - fig.ymin) / (fig.ymax - fig.ymin) * (h - 2 * margin);

    // Ensure points stay within graph boundaries
    sx = std::max(margin, std::min(w - margin, sx));
    sy = std::max(margin, std::min(h - margin, sy));

    return sf::Vector2f(static_cast<float>(sx), static_cast<float>(sy));
}

void PlotGen::draw_text(const Figure &fig, double w, double h)
{
    sf::Text text;
    text.setFont(font);
    text.setFillColor(sf::Color::Black);

    // Titre avec une taille de police réduite
    text.setCharacterSize(18);
    text.setString(sf::String::fromUtf8(fig.title.begin(), fig.title.end()));
    sf::FloatRect textRect = text.getLocalBounds();
    
    // Positionner le titre en dehors de la zone de dessin
    double margin = 50.0f;
    text.setPosition(w / 2 - textRect.width / 2, margin / 2 - textRect.height / 2);
    texture.draw(text);

    // X label avec police plus petite
    text.setCharacterSize(14);
    text.setString(sf::String::fromUtf8(fig.xlabel.begin(), fig.xlabel.end()));
    textRect = text.getLocalBounds();
    text.setPosition(w / 2 - textRect.width / 2, h - 20);
    texture.draw(text);

    // Y label avec police plus petite
    text.setCharacterSize(14);
    text.setString(sf::String::fromUtf8(fig.ylabel.begin(), fig.ylabel.end()));
    textRect = text.getLocalBounds();
    text.setRotation(-90);
    text.setPosition(10, h / 2 + textRect.width / 2);
    texture.draw(text);
    text.setRotation(0);

    // Légende
    if (fig.show_leg && !fig.curves.empty()) {
        const double max_legend_width = 130.0f; // Largeur maximale du texte de légende
        double padding_x = 10.0f; // Marge horizontale
        double padding_y = 8.0f;  // Marge verticale
        
        // Collecter toutes les légendes et calculer les dimensions
        std::vector<std::pair<const Figure::Curve*, std::vector<std::string>>> legend_items;
        double total_legend_height = 0.0f;
        double max_content_width = 0.0f;

        // Première passe: collecter et préparer les légendes
        for (const auto &curve : fig.curves) {
            if (!curve.style.legend.empty()) {
                std::vector<std::string> legend_lines;
                std::string legend_text = curve.style.legend;
                
                text.setCharacterSize(12);
                text.setString(sf::String::fromUtf8(legend_text.begin(), legend_text.end()));
                sf::FloatRect bounds = text.getLocalBounds();

                // Couper le texte si trop long
                if (bounds.width > max_legend_width) {
                    // Découper en mots
                    std::vector<std::string> words;
                    std::string current_word;
                    for (char c : legend_text) {
                        if (c == ' ' || c == '\t' || c == '\n') {
                            if (!current_word.empty()) {
                                words.push_back(current_word);
                                current_word.clear();
                            }
                            if (c == '\n') words.push_back("\n");
                        } else {
                            current_word += c;
                        }
                    }
                    if (!current_word.empty()) words.push_back(current_word);

                    // Construire des lignes
                    std::string current_line;
                    for (const std::string &word : words) {
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

                // Ajouter à notre collection
                legend_items.push_back({&curve, legend_lines});
                
                // Calculer la hauteur pour cette entrée
                double item_height = 24 + (legend_lines.size() - 1) * 16; // 24px pour la première ligne, 16px pour les suivantes
                total_legend_height += item_height;
                
                // Calculer la largeur maximale du contenu
                for (const auto &line : legend_lines) {
                    text.setString(sf::String::fromUtf8(line.begin(), line.end()));
                    double line_width = text.getLocalBounds().width;
                    max_content_width = std::max(max_content_width, line_width);
                }
            }
        }
        
        // Si nous avons des éléments à afficher
        if (!legend_items.empty()) {
            // Calculer la taille exacte et la position du rectangle de la légende
            double legend_width = max_content_width + 40.0f + 2 * padding_x; // 40px pour l'exemple de style
            double legend_height = total_legend_height + 2 * padding_y;
            
            double legend_x, legend_y;
            
            // Déterminer la position en fonction de fig.legend_position
            if (fig.legend_position == "top-right") {
                legend_x = w - legend_width - padding_x;
                legend_y = padding_y + margin;
            } 
            else if (fig.legend_position == "top-left") {
                legend_x = margin + padding_x;
                legend_y = padding_y + margin;
            }
            else if (fig.legend_position == "bottom-right") {
                legend_x = w - legend_width - padding_x;
                legend_y = h - legend_height - padding_y - margin;
            }
            else if (fig.legend_position == "bottom-left") {
                legend_x = margin + padding_x;
                legend_y = h - legend_height - padding_y - margin;
            }
            else if (fig.legend_position == "outside-right") {
                // Placer la légende à l'extérieur du graphique à droite
                legend_x = w + padding_x;
                legend_y = margin + padding_y;
                
                // Ajuster la vue pour que la légende soit visible
                sf::View current_view = texture.getView();
                sf::FloatRect viewport = current_view.getViewport();
                
                // Élargir la vue pour inclure la légende
                viewport.width += legend_width / w;
                current_view.setViewport(viewport);
                texture.setView(current_view);
            }
            else {
                // Position par défaut: top-right
                legend_x = w - legend_width - padding_x;
                legend_y = padding_y + margin;
            }
            
            // Dessiner l'arrière-plan de la légende
            sf::RectangleShape legendBg;
            legendBg.setSize(sf::Vector2f(legend_width, legend_height));
            legendBg.setPosition(legend_x, legend_y);
            legendBg.setFillColor(sf::Color(255, 255, 255, 220));
            legendBg.setOutlineColor(sf::Color::Black);
            legendBg.setOutlineThickness(1.0f);
            texture.draw(legendBg);
            
            // Dessiner les éléments de la légende
            double current_y = legend_y + padding_y;
            
            for (const auto &item : legend_items) {
                const Figure::Curve* curve = item.first;
                const std::vector<std::string> &legend_lines = item.second;
                
                // Dessiner l'exemple de style
                float sample_width = 30.0f;
                // float sample_height = 16.0f;
                float start_x = legend_x + padding_x;
                float mid_y = current_y + 8.0f;
                
                // Style de ligne
                if (curve->style.line_style == "solid") {
                    float thickness = std::max(1.0f, static_cast<float>(curve->style.thickness));
                    sf::RectangleShape line(sf::Vector2f(sample_width, thickness));
                    line.setPosition(start_x, mid_y - thickness/2);
                    line.setFillColor(curve->style.color);
                    texture.draw(line);
                } 
                else if (curve->style.line_style == "dashed") {
                    float dash_length = 6.0f;
                    float thickness = std::max(1.0f, static_cast<float>(curve->style.thickness));
                    for (int i = 0; i < 3; i++) {
                        sf::RectangleShape dash(sf::Vector2f(dash_length, thickness));
                        dash.setPosition(start_x + i * 2 * dash_length, mid_y - thickness/2);
                        dash.setFillColor(curve->style.color);
                        texture.draw(dash);
                    }
                }
                
                // Symbole
                if (curve->style.symbol_type != "none") {
                    sf::Vector2f symbol_pos(start_x + sample_width/2, mid_y);
                    draw_symbol(symbol_pos, curve->style.symbol_type, curve->style.symbol_size, curve->style.color);
                }
                
                // Texte de la légende
                text.setCharacterSize(12);
                float text_y = current_y;
                for (const auto &line : legend_lines) {
                    text.setString(sf::String::fromUtf8(line.begin(), line.end()));
                    text.setPosition(start_x + sample_width + 10, text_y);
                    texture.draw(text);
                    text_y += 16;
                }
                
                // Passer à l'élément suivant
                current_y += 24 + (legend_lines.size() - 1) * 16;
            }
        }
    }
}

// Method for drawing a symbol at a given position
void PlotGen::draw_symbol(const sf::Vector2f &position, const std::string &symbol_type, double size, const sf::Color &color)
{
    if (symbol_type == "none")
        return;

    if (symbol_type == "circle")
    {
        sf::CircleShape circle(size / 2);
        circle.setOrigin(size / 2, size / 2);
        circle.setPosition(position);
        circle.setFillColor(color);
        circle.setOutlineColor(sf::Color::Black);
        circle.setOutlineThickness(1.0f);
        texture.draw(circle);
    }
    else if (symbol_type == "square")
    {
        sf::RectangleShape square(sf::Vector2f(size, size));
        square.setOrigin(size / 2, size / 2);
        square.setPosition(position);
        square.setFillColor(color);
        square.setOutlineColor(sf::Color::Black);
        square.setOutlineThickness(1.0f);
        texture.draw(square);
    }
    else if (symbol_type == "triangle")
    {
        sf::CircleShape triangle(size / 2, 3); // triangle = circle with 3 sides
        triangle.setOrigin(size / 2, size / 2);
        triangle.setPosition(position);
        triangle.setFillColor(color);
        triangle.setOutlineColor(sf::Color::Black);
        triangle.setOutlineThickness(1.0f);
        texture.draw(triangle);
    }
    else if (symbol_type == "diamond")
    {
        sf::CircleShape diamond(size / 2, 4); // diamond = circle with 4 sides
        diamond.setOrigin(size / 2, size / 2);
        diamond.setPosition(position);
        diamond.setRotation(45.0f); // 45° rotation to get a diamond
        diamond.setFillColor(color);
        diamond.setOutlineColor(sf::Color::Black);
        diamond.setOutlineThickness(1.0f);
        texture.draw(diamond);
    }
    else if (symbol_type == "star")
    {
        // Create a 5-pointed star
        const int numPoints = 5;
        const double innerRadius = size / 4;
        const double outerRadius = size / 2;

        sf::ConvexShape star;
        star.setPointCount(numPoints * 2);
        star.setOrigin(size / 2, size / 2);
        star.setPosition(position);
        star.setFillColor(color);
        star.setOutlineColor(sf::Color::Black);
        star.setOutlineThickness(1.0f);

        for (int i = 0; i < numPoints * 2; i++)
        {
            double radius = (i % 2 == 0) ? outerRadius : innerRadius;
            double angle = i * 3.14159f / numPoints;
            sf::Vector2f point(radius * std::cos(angle), radius * std::sin(angle));
            star.setPoint(i, point);
        }

        texture.draw(star);
    }
}