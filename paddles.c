#include <gtk/gtk.h>

#define PADDLE_STEP 5               // Step of a paddle in pixels
#define PADDLE_PERIOD 5             // Period of a paddle in milliseconds
#define DISC_PERIOD 4               // Period of the disc in milliseconds
#define END_GAME_SCORE 5            // Maximum number of points for a player

// State of the game.
typedef enum State
{
    STOP,                           // Stop state
    PLAY,                           // Play state
    PAUSE,                          // Pause state
} State;

// Structure of a player.
typedef struct Player
{
    GdkRectangle rect;              // Position and size of the player's paddle
    gint step;                      // Vertical step of the player's paddle in pixels
    guint score;                    // Score
    GtkLabel* label;                // Label used to display the score
    guint event;                    // Event ID used to move the paddle
} Player;

// Structure of the disc.
typedef struct Disc
{
    GdkRectangle rect;              // Position and size
    GdkPoint step;                  // Horizontal and verical steps in pixels
    guint period;                   // Period in milliseconds
    guint event;                    // Event ID used to move the disc
} Disc;

// Structure of the graphical user interface.
typedef struct UserInterface
{
    GtkWindow* window;              // Main window
    GtkDrawingArea* area;           // Drawing area
    GtkButton* start_button;        // Start button
    GtkButton* stop_button;         // Stop button
    GtkScale* speed_scale;          // Speed scale
    GtkCheckButton* training_cb;    // Training check box
} UserInterface;

// Structure of the game.
typedef struct Game
{
    State state;                    // State of the game
    Player p1;                      // Player 1
    Player p2;                      // Player 2
    Disc disc;                      // Disc
    UserInterface ui;               // User interface
} Game;



// Event handler for the "draw" signal of the drawing area.
gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    // Gets the 'Game' structure.
    Game *game = user_data;

    // Sets the background to white.
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // PLayer 1 Paddle 
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, game->p1.rect.x, game->p1.rect.y, 
            game->p1.rect.width, game->p1.rect.height);
    cairo_fill(cr);

     // PLayer 2 Paddle 
    cairo_rectangle(cr, game->p2.rect.x, game->p2.rect.y, 
            game->p2.rect.width, game->p2.rect.height);
    cairo_fill(cr);
    
    // Draws the disc in red.
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_rectangle(cr, game->disc.rect.x, game->disc.rect.y,
        game->disc.rect.width, game->disc.rect.height);
    cairo_fill(cr);

    // Propagates the signal.
    return FALSE;
}

// Redraws an item in the drawing area.
//
// Parameters:
//
// * `area`: Drawing area.
// * `old`: Previous position of the item to redraw.
// * `new`: New position of the item to redraw.
void redraw_item(GtkDrawingArea *area, GdkRectangle *old, GdkRectangle *new)
{
    // Determines the part of the area to redraw.
    // (The union of the previous and new positions of the disc.)
    gdk_rectangle_union(old, new, old);

    // Redraws the disc.
    gtk_widget_queue_draw_area((GtkWidget*)area,
        old->x, old->y, old->width, old->height);

}

// Timeout function called at regular intervals to move the disc.
gboolean on_move_disc(gpointer user_data)
{
    // Gets the `Game` structure passed as parameter.
    Game* game = user_data;

    // Gets the largest coordinate for the disc.
    gint x_max = gtk_widget_get_allocated_width(GTK_WIDGET(game->ui.area))
        - game->disc.rect.width;
    gint y_max = gtk_widget_get_allocated_height(GTK_WIDGET(game->ui.area))
        - game->disc.rect.height;

    // Gets the current position of the disc.
    GdkRectangle old = game->disc.rect;

    // Works out the new position of the disc.
    game->disc.rect.x = CLAMP(game->disc.rect.x + game->disc.step.x, 0, x_max);
    game->disc.rect.y = CLAMP(game->disc.rect.y + game->disc.step.y, 0, y_max);

    if (game->disc.rect.y >= y_max || game->disc.rect.y == 0)
       game->disc.step.y *= -1; 
    if (game->disc.rect.x >= x_max || game->disc.rect.x == 0)
       game->disc.step.x *= -1; 
    // Redraws the disc.
    redraw_item(game->ui.area, &old, &game->disc.rect);

    // Enables the next call.
    return TRUE;
}

// Sets the 'Play' state.
void set_play(Game* game)
{
    // - Set the state field to PLAY.
    game->state = PLAY;

    // - Set the label of the start button to "Pause".
    gtk_button_set_label(game->ui.start_button, "Pause");
    
    // - Disable the stop button.
    gtk_widget_set_sensitive(GTK_WIDGET(game->ui.stop_button), FALSE);
    
    // - Set the on_move_disc() function to be called at regular intervals.
    game->disc.event = g_timeout_add(game->disc.period, on_move_disc, game);
}

// Sets the 'Pause' state.
void set_pause(Game* game)
{
    // - Set the state field to PAUSE.
    game->state = PAUSE;

    // - Set the label of the start button to "Resume".
    gtk_button_set_label(game->ui.start_button, "Resume");
    
    // - Enable the stop button.
    gtk_widget_set_sensitive(GTK_WIDGET(game->ui.stop_button), TRUE);
    
    // - Stop the on_move_disc() function.
    g_source_remove(game->disc.event);
}

// Sets the 'Stop' state.
void set_stop(Game *game)
{
    // - Set the state field to STOP.
    game->state = STOP;

    // - Set the label of the start button to "Start".
    gtk_button_set_label(game->ui.start_button, "Start");
    
    // - Disable the stop button.
    gtk_widget_set_sensitive(GTK_WIDGET(game->ui.stop_button), FALSE);
}

// Event handler for the "clicked" signal of the start button.
void on_start(GtkButton *button, gpointer user_data)
{
    // Gets the `Game` structure.
    Game *game = user_data;

    // Sets the next state according to the current state.
    switch (game->state)
    {
        case STOP: set_play(game); break;
        case PLAY: set_pause(game); break;
        case PAUSE: set_play(game); break;
    };
}

// Event handler for the "clicked" signal of the stop button.
void on_stop(GtkButton *button, gpointer user_data)
{
    set_stop(user_data);
}

gboolean on_move_paddle_one(gpointer user_data)
{
    Game* game = user_data;

    gint y_max = gtk_widget_get_allocated_height(GTK_WIDGET(game->ui.area))
        - game->p1.rect.height;

    GdkRectangle old = game->p1.rect;

    game->p1.rect.y = CLAMP(game->p1.rect.y + game->p1.step, 0, y_max);

    redraw_item(game->ui.area, &old, &game->p1.rect);

    return TRUE;

}

gboolean on_move_paddle_two(gpointer user_data)
{
    Game* game = user_data;

    gint y_max = gtk_widget_get_allocated_height(GTK_WIDGET(game->ui.area))
        - game->p2.rect.height;

    GdkRectangle old = game->p2.rect;

    game->p2.rect.y = CLAMP(game->p2.rect.y + game->p2.step, 0, y_max);

    redraw_item(game->ui.area, &old, &game->p2.rect);

    return TRUE;

}


// Event handler for the "key-press-event" signal.
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    Game* game = user_data;
    
    // If the 'f' key is pressed, moves the player 1 upwards.
    if (event->keyval == GDK_KEY_f)
    {
        game->p1.step = -1*PADDLE_STEP;
        if (game->p1.event == 0)
            game->p1.event = g_timeout_add(PADDLE_PERIOD, on_move_paddle_one, game);
        return TRUE;
    }

    // If the 'v' key is pressed, moves the player 1 downwards.
    else if (event->keyval == GDK_KEY_v)
    {
        game->p1.step = PADDLE_STEP;
        if (game->p1.event == 0)
            game->p1.event = g_timeout_add(PADDLE_PERIOD, on_move_paddle_one, game);
        return TRUE;
    }

    // If the 'Up Arrow' key is pressed, moves the player 2 upwards.
    else if (event->keyval == GDK_KEY_Up)
    {
        game->p2.step = -1*PADDLE_STEP;
        if (game->p2.event == 0)
            game->p2.event = g_timeout_add(PADDLE_PERIOD, on_move_paddle_two, game);
        return TRUE;
    }

    // If the 'Down Arrow' key is pressed, moves the player 2 downwards.
    else if (event->keyval == GDK_KEY_Down)
    {
        game->p2.step = PADDLE_STEP;
        if (game->p2.event == 0)
            game->p2.event = g_timeout_add(PADDLE_PERIOD, on_move_paddle_two, game);
        return TRUE;
    }

    // Otherwise, propagates the signal.
    else
        return FALSE;
}

// Event handler for the "key-release-event" signal.
gboolean on_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    Game* game = user_data;
    // If the 'f' key is released, stops the player 1.
    if (event->keyval == GDK_KEY_f)
    {
        if (game->p1.step < 0) {
            g_source_remove(game->p1.event);
            game->p1.event = 0;
        }
        return TRUE;
    }

    // If the 'v' key is released, stops the player 1.
    if (event->keyval == GDK_KEY_v)
    {
        if (game->p1.step > 0) {
            g_source_remove(game->p1.event);
            game->p1.event = 0;
        }
        return TRUE;
    }

    // If the 'Up Arrow' key is released, stops the player 2.
    if (event->keyval == GDK_KEY_Up)
    {
        if (game->p2.step < 0) {
            g_source_remove(game->p2.event);
            game->p2.event = 0;
        }
        return TRUE;
    }

    // If the 'Down Arrow' key is released, stops the player 2.
    if (event->keyval == GDK_KEY_Down)
    {
        if (game->p2.step > 0) {
            g_source_remove(game->p2.event);
            game->p2.event = 0;
        }
        return TRUE;
    }

    // Otherwise, propagates the signal.
    else
        return FALSE;
}


int main (int argc, char *argv[])
{
    // Initializes GTK.
    gtk_init(NULL, NULL);

    // Constructs a GtkBuilder instance.
    GtkBuilder* builder = gtk_builder_new ();

    // Loads the UI description.
    // (Exits if an error occurs.)
    GError* error = NULL;
    if (gtk_builder_add_from_file(builder, "duel.glade", &error) == 0)
    {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    // Gets the widgets.
    GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.duel"));
    GtkDrawingArea* area = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "area"));
    GtkButton* start_button = GTK_BUTTON(gtk_builder_get_object(builder, "start_button"));
    GtkButton* stop_button = GTK_BUTTON(gtk_builder_get_object(builder, "stop_button"));
    GtkLabel* p1_score_label = GTK_LABEL(gtk_builder_get_object(builder, "p1_score_label"));
    GtkLabel* p2_score_label = GTK_LABEL(gtk_builder_get_object(builder, "p2_score_label"));
    GtkScale* speed_scale = GTK_SCALE(gtk_builder_get_object(builder, "speed_scale"));
    GtkCheckButton* training_cb = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "training_cb"));

    // Creates the "Game" structure.
    Game game =
    {
        .state = STOP,

        .p1 =
            {
                .rect = { 0, 0, 10, 100 },
                .step = PADDLE_STEP,
                .score = 0,
                .label = p1_score_label,
                .event = 0,
            },

        .p2 =
            {
                .rect = { 800 - 10, 0, 10, 100 },
                .step = PADDLE_STEP,
                .score = 0,
                .label = p2_score_label,
                .event = 0,
            },

        .disc =
            {
                .rect = { 100, 100, 10, 10 },
                .step = { 1, 1 },
                .event = 0,
                .period = DISC_PERIOD,
            },

        .ui =
            {
                .window = window,
                .area = area,
                .start_button = start_button,
                .stop_button = stop_button,
                .speed_scale = speed_scale,
                .training_cb = training_cb,
            },
    };

    // Connects event handlers.
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(area, "draw", G_CALLBACK(on_draw), &game);
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start), &game);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop), &game);
    g_signal_connect(window, "key_press_event", G_CALLBACK(on_key_press), &game);
    g_signal_connect(window, "key_release_event", G_CALLBACK(on_key_release), &game); 
    
    // Runs the main loop.
    gtk_main();

    // Exits.
    return 0;
}


