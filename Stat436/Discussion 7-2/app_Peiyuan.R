#Lib all required packages
library(shiny)
library(tidyverse)
library(plotly)
library(RColorBrewer)

# Load and prepare data outside of the server function for global access
data <- read_csv("https://raw.githubusercontent.com/pli233/datasets/main/overall_player_stats.csv") %>%
  mutate(HSP = as.numeric(str_replace(HSP, "%", "")) / 100)  # Convert HSP from percentage strings to decimals

# Custom CSS for app aesthetics
customCSS <- "
@import url('https://fonts.googleapis.com/css2?family=Nunito:wght@400;600;700&display=swap');

.centered-title {
  text-align: center;
  width: 100%;
  font-size: 24px; /* Adjust font size as needed */
  font-weight: bold;
  margin: 20px 0; /* Adjust top and bottom margin as needed */
  font-family: 'Nunito', sans-serif; /* Ensure font consistency */
}

.body-content {
  margin: 10px;
  padding: 10px;
  font-family: 'Nunito', sans-serif;
  background-color: #f4f4f4;
}

.title {
  color: #34495e;
  font-size: 26px;
  font-weight: 700;
  text-align: center;
  margin-bottom: 30px;
}

.sidebar {
  background-color: #ffffff;
  border-right: 2px solid #eaeaea;
}

.sidebar-panel .well {
  background-color: #f9f9f9;
  border: none;
  box-shadow: 0 2px 4px 0 rgba(0, 0, 0, 0.05);
  padding: 15px;
}

.main-panel {
  padding: 20px;
  background-color: #ffffff;
  border-left: 2px solid #eaeaea;
}

.selectize-input, .selectize-dropdown {
  border-radius: 4px;
  border: 1px solid #ced4da;
  box-shadow: inset 0 1px 1px rgba(0,0,0,.075);
  transition: border-color ease-in-out .15s, box-shadow ease-in-out .15s;
}

.selectize-input:focus, .selectize-dropdown:focus {
  border-color: #80bdff;
  outline: 0;
  box-shadow: 0 0 0 0.2rem rgba(0,123,255,.25);
}

.plot-container {
  text-align: center;
  background-color: #ffffff;
  border: 1px solid #ddd;
  margin-top: 20px;
  border-radius: 4px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

/* Responsive design adjustments */
@media (max-width: 768px) {
  .sidebar, .main-panel {
    padding: 10px;
    border-right: none;
    border-left: none;
  }
}

/* Additional styles for buttons, links, etc. */
button {
  background-color: #3498db;
  color: white;
  border: none;
  padding: 10px 15px;
  border-radius: 4px;
  transition: background-color 0.3s ease;
}

button:hover {
  background-color: #2980b9;
}

a {
  color: #3498db;
  text-decoration: none;
}

a:hover {
  text-decoration: underline;
}

"

# Define UI
ui <- fluidPage(
  tags$head(
    tags$style(HTML(customCSS))  # Include the custom CSS for styling
  ),
  div(HTML("<br>STAT436 HW2 SHINY PORTFOLIO PROJECT <br>Peiyuan Li <br> 2024-3-10")),
  div(class="centered-title", "Valorant Players' Stats Visualization"),  # Custom div for centered title

  # Tabs to switch between radar charts and bar plots, with added styling
  tabsetPanel(
    #Radar chart Panel
    tabPanel("Radar Charts",
             div(
               div(class = "title", "Player Radar Charts"),
               class = "body-content",
               sidebarLayout(
                 sidebarPanel(
                   selectInput('selectedPlayer', 'Select a player:', choices = paste(data$Team, data$Player, sep=" ")),
                   tags$div(class = "sidebar")
                 ),
                 mainPanel(
                   plotlyOutput("radarPlot"),
                   div(class = "plot-container")
                 )
               )
             )),
    
    # Bar plot banel
    tabPanel("Bar Plots",
             div(
               div(class = "title", "Player Stats Comparison"),
               fluidRow(
                 div(
                   class = "sidebar-panel",
                   column(6,
                          selectInput("stat",
                                      "Choose a stat to display:",
                                      choices = colnames(data)[4:ncol(data)],
                                      selectize = TRUE)
                   ),
                   column(6,
                          selectInput("topN",
                                      "Select number of top players to display:",
                                      choices = c("Top 10" = 10, "Top 20" = 20, "All" = nrow(data)),
                                      selected = 10,
                                      selectize = TRUE)
                   )
                 )
               ),
               div(
                 class = "main-panel",
                 plotlyOutput("statPlot"),
                 div(class = "plot-container")
               )
             ))
  )
)

# Server logic
server <- function(input, output) {
  
  # Radar Plot for selected player
  output$radarPlot <- renderPlotly({
    req(input$selectedPlayer)  # Require that a player is selected
    
    # Split the combined name back into team and player
    selected_team_player <- str_split(input$selectedPlayer, " ", simplify = TRUE)
    selected_team <- selected_team_player[1]
    selected_player <- selected_team_player[2]
    
    # Filter the data based on both team and player to get the right stats
    selected_data <- data %>%
      filter(Team == selected_team, Player == selected_player) %>%
      select(ACS, KD, ADR, KPR, HSP, FKPR, FDPR) %>%
      mutate(FDPR_FKPR = FDPR - FKPR) %>%
      pivot_longer(cols = everything(), names_to = "Metric", values_to = "Value")
    
    # Get the max values to normalize the radar plot
    max_values <- data %>%
      summarise(across(c(ACS, KD, ADR, KPR, HSP, FKPR, FDPR), max, na.rm = TRUE))
    
    # Normalize the metrics for the selected player
    selected_data_normalized <- selected_data
    selected_data_normalized$Value <- map2(selected_data_normalized$Metric, selected_data_normalized$Value, function(metric, value) {
      value / max_values[[metric]]
    })
    
    # Plot the radar chart
    p <- plot_ly(
      selected_data_normalized,
      type = 'scatterpolar',
      fill = 'toself',
      r = ~Value,
      theta = ~Metric
    ) %>%
      layout(
        polar = list(
          radialaxis = list(
            visible = T,
            range = c(0, 1)
          )
        ),
        title = list(text = paste("Player Stats:", input$selectedPlayer), x = 0.5),
        showlegend = F,
        margin = list(t = 75) # Adjust top margin to provide space for the title
      )
    p
  })
  
  
  
  # Bar Plot for stat comparison among top N players
  output$statPlot <- renderPlotly({
    req(input$stat, input$topN) # Make sure input$stat and input$topN are available before proceeding
    
    # Combine the Team and Player columns and order the data based on the selected stat in descending order
    ordered_data <- data %>%
      mutate(CombinedName = paste(Team, Player, sep = " ")) %>%
      arrange(desc(.data[[input$stat]])) %>%
      mutate(CombinedName = factor(CombinedName, levels = CombinedName))  # Reorder factor levels for the plot
    
    # Determine the number of players to display and adjust title accordingly
    title_text <- if (input$topN == nrow(data)) {
      "All"
    } else {
      paste("Top", input$topN)
    }
    
    # Limit the data to the top N players if not "All"
    limited_data <- if (title_text == "All") {
      ordered_data
    } else {
      head(ordered_data, input$topN)
    }
    
    # Generate a color palette with a smooth gradient
    color_palette <- colorRampPalette(c("#EE4B2B", "#ff7f0e", "#2ca02c", "#1f77b4", "#7393B3"))(length(unique(limited_data$CombinedName)))
    
    # Generate the plot
    p <- ggplot(limited_data, aes(x = CombinedName, y = .data[[input$stat]], fill = CombinedName)) +
      geom_bar(stat = "identity") +
      scale_fill_manual(values = color_palette) +
      theme_minimal() +
      theme(
        axis.text.x = element_text(angle = 45, vjust = 1, hjust=1),
        legend.position = "none",
        plot.title = element_text(hjust = 0.5)  # Center the plot title
      ) +
      labs(x = "Player", y = input$stat, title = paste(input$stat, "(", title_text, ")"))
    
    # Convert to plotly for an interactive plot
    ggplotly(p, tooltip = c("x", "y"))
  })
  
}

# Run the app
shinyApp(ui = ui, server = server)
