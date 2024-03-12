library(shiny)
library(plotly)
library(tidyverse)
library(rsconnect)

ts <- read_csv("https://uwmadison.box.com/shared/static/dr3jgcqcd353i6dcegzup1azlpi4kh6k.csv")
ts <- ts %>% select("name", "album","release_date", "popularity","danceability","acousticness","energy", "liveness", "valence") %>% mutate(popularity = popularity * 0.01)

ui <- fluidPage(
  titlePanel("Taylor Swift Spotify Data"),
  sidebarLayout(
    sidebarPanel(
      selectInput("album", "Choose Album:", choices = unique(ts$album)),
      selectInput("song", "Choose Song:", choices = NULL),
      textOutput("albumReleaseDate")
    ),
    mainPanel(
      plotlyOutput("featurePlot")
    )
  )
)

server <- function(input, output, session) {
  
  observe({
    updateSelectInput(session, "song",
                      choices = ts %>% filter(album == input$album) %>% `$`(name))
  })
  
  # Show the release date
  output$albumReleaseDate <- renderText({
    release_date <- ts %>% 
      filter(album == input$album) %>% 
      summarise(release_date = unique(release_date)) %>% 
      pull(release_date)
    format(as.Date(release_date), "%Y-%m-%d")
  })
  
  output$featurePlot <- renderPlotly({
    req(input$song)
    
    feature_order <- c("popularity", "danceability", "acousticness", "energy", "liveness", "valence")
    
    selected_song_features <- ts %>% filter(name == input$song) %>% select(all_of(feature_order))
    
    album_mean_features <- ts %>% filter(album == input$album) %>% summarise(across(all_of(feature_order), mean))
    
    features_combined <- bind_rows(Selected_Song = selected_song_features, Album_Mean = album_mean_features) %>%
      pivot_longer(cols = feature_order, names_to = "feature", values_to = "value") %>%
      mutate(feature = factor(feature, levels = feature_order),
             Song_or_Album = rep(c("Selected Song", "Album Mean"), each = length(feature_order)))
    
    plot_ly(data = features_combined, x = ~feature, y = ~value, color = ~Song_or_Album, type = 'scatter', mode = 'lines+markers',
            line = list(width = 2), marker = list(size = 10, opacity = 0.7, color = ifelse(features_combined$Song_or_Album == "Selected Song", "blue", "grey"))) %>%
      layout(title = "Feature Comparison: Selected Song vs. Album Mean",
             xaxis = list(title = "Feature"),
             yaxis = list(title = "Value"))
  })
}


shinyApp(ui, server)