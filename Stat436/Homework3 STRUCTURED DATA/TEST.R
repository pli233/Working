library(shiny)
library(tidyverse)
olympics <- read_csv("https://uwmadison.box.com/shared/static/rzw8h2x6dp5693gdbpgxaf2koqijo12l.csv")
#' Scatterplot with highlighted points
#'
#' Assumes a column in df called "selected" saying whether points should be
#' larger / darker
scatterplot <- function(df) {
  ggplot(df) +
    geom_point(aes(Weight, `Height, cm`,alpha = as.numeric(selected),size = as.numeric(selected))) +
    scale_alpha(range = c(0.05, .8)) +
    scale_size(range = c(0.1, 1))
}
ui <- fluidPage(
  selectInput("dropdown", "Select a Sport", choices = unique(olympics$Sport), multiple = TRUE),
  plotOutput("scatterplot")
)
server <- function(input, output) {
  # Reactive expression to filter data based on selected sports
  filtered_data <- reactive({
    if (length(input$dropdown) == 0) {
      olympics$selected <- FALSE
    } else {
      olympics$selected <- olympics$Sport %in% input$dropdown
    }
    olympics
  })
  
  # Render the scatterplot based on filtered data
  output$scatterplot <- renderPlot({
    scatterplot(filtered_data())
  })
}