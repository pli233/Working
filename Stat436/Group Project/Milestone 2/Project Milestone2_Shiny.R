#1. Include all required library
# Required Libraries
library(tidyverse)
library(dplyr)
library(tmap)
library(treemap)
library(ggplot2)
library(devtools)
library(d3treeR)
library(plotly)
library(shiny)
library(shinydashboard) 
library(shinyWidgets) 

# Preprocess Dataset Function
preprocess_dataset <- function(data) {
  data %>%
    separate(year, sep = "/", into = c("year"), convert = TRUE, extra = "drop")
}

# Load and Preprocess Data
source_of_fund <- preprocess_dataset(read_csv("https://raw.githubusercontent.com/pli233/Working/main/Stat436/Group%20Project/data/source_of_fund.csv"))
field_of_study <- preprocess_dataset(read_csv("https://raw.githubusercontent.com/pli233/Working/main/Stat436/Group%20Project/data/field_of_study.csv"))
origin <- preprocess_dataset(read_csv("https://raw.githubusercontent.com/pli233/Working/main/Stat436/Group%20Project/data/origin.csv"))
academic_detail <- preprocess_dataset(read_csv("https://raw.githubusercontent.com/pli233/Working/main/Stat436/Group%20Project/data/academic_detail.csv"))



#map data
data("World")
world <- World |>
  select(c('sovereignt',"geometry")) |>
  rename('origin' = 'sovereignt')

test <- world |>
  inner_join(origin |> filter(academic_type == "Undergraduate"), by = 'origin')

#3.4 custom break
custom_breaks <- function(x) {
  c(
    min(x),       
    min(x)+(max(x)-min(x))/20,
    min(x)+(max(x)-min(x))/15,
    min(x)+(max(x)-min(x))/10,
    min(x)+(max(x)-min(x))/5,
    min(x)+(max(x)-min(x))/3,
    min(x)+(max(x)-min(x))/2,
    max(x)       
  )
}


#4. Customized Plotting Functions

#4.1 pie graph generator
pie <- function(df) {
  filtered_df <- df %>%
    filter(selected) %>%
    group_by(source_of_fund) %>%
    summarize(students = sum(students)) %>%
    mutate(percentage = round(students / sum(students) * 100, 1))  # Calculate percentages
  
  plot_ly(filtered_df, labels = ~source_of_fund, values = ~students, type = 'pie',
          textinfo = 'label+percent', hoverinfo = 'label+percent',
          text = ~paste(source_of_fund, ": ", percentage, "%"),
          marker = list(colors = RColorBrewer::brewer.pal(8, "Set3"))) 
}


#4.2 map graph generator
map <- function(df) {
  filtered_df <- df %>% filter(selected)
  breaks <- custom_breaks(filtered_df$students)
  tm_shape(World) +
    tm_polygons(col = "white") +
    tm_shape(filtered_df) +
    tm_polygons("students", breaks = breaks, 
                title = "Number of Students") +
    tm_layout(legend.title.size = 0.8, legend.text.size = 0.6)
}





# Shiny UI
# UI Setup
ui <- dashboardPage(
  dashboardHeader(title = "International Education in the US"),
  dashboardSidebar(
    sidebarMenu(
      menuItem("Filter Options", tabName = "filters", icon = icon("filter"), startExpanded = TRUE,
               sliderInput("yearRange", "Year of Release:", min = 1990, max = 2022, value = c(1990, 2022), step = 1)),
      menuItem("Source of Fund", tabName = "fundSource", icon = icon("money-check-dollar")),
      menuItem("Student Origin", tabName = "studentOrigin", icon = icon("globe")),
      menuItem("Field Of Study", tabName = "fieldOfStudy", icon = icon("chart-line")),
      menuItem("Students Type", tabName = "studentsType", icon = icon("user-graduate"))
    )
  ),
  dashboardBody(
    tabItems(
      tabItem(tabName = "filters", h3("Use the slider to select the year range for your analysis."), tags$hr()),
      tabItem(tabName = "fundSource", plotlyOutput("pie")),
      tabItem(tabName = "studentOrigin", plotOutput("map")),
      tabItem(tabName = "fieldOfStudy", fluidPage(
        sidebarLayout(
          sidebarPanel(
            selectInput("yAxisType", "Select Y-Axis Data Type:", choices = c("Absolute Number" = "absolute", "Percentage" = "percentage")),
            uiOutput("fieldCheckbox")
          ),
          mainPanel(plotlyOutput("interactivePlot"))
        )
      )),
      tabItem(tabName = "studentsType", fluidPage(
        titlePanel("Students Type Over Years"),
        mainPanel(plotlyOutput("studentTypePlot"))
      ))
    )
  )
)


# Server Logic
server <- function(input, output) {
  # Reactive expression for filtering datasets based on yearRange
  subset_by_year <- reactive({
    list(
      fund_subset = source_of_fund %>% mutate(selected = (year >= input$yearRange[1]) & (year <= input$yearRange[2])),
      test_subset = test %>% mutate(selected = (year >= input$yearRange[1]) & (year <= input$yearRange[2])),
      field_of_study_subset = field_of_study %>% mutate(selected = (year >= input$yearRange[1]) & (year <= input$yearRange[2])),
      academic_detail_subset =  academic_detail %>% filter(year >= input$yearRange[1] & year <= input$yearRange[2])
    )
  })
  
  # Plots and UI rendering
  output$pie <- renderPlotly({ pie(subset_by_year()$fund_subset) })
  
  
  
  output$map <- renderPlot({
    map(subset_by_year()$test_subset)
  })
  
  # Interactive plot logic
  output$fieldCheckbox <- renderUI({
    fields <- unique(subset_by_year()$field_of_study_subset$field_of_study)
    checkboxGroupInput("selectedFields", "Select Fields of Study:", choices = fields, selected = fields)
  })
  
  data_to_plot <- reactive({
    data <- subset_by_year()$field_of_study_subset %>%
      filter(!is.na(students), field_of_study %in% input$selectedFields)
    
    if (input$yAxisType == "percentage") {
      data <- data %>%
        group_by(year, major) %>%
        summarise(students_sum = sum(students), .groups = 'drop') %>%
        group_by(year) %>%
        mutate(total_students = sum(students_sum),
               percentage = students_sum / total_students * 100,
               students = students_sum) %>%  # Ensure 'students' column is present for tooltip
        ungroup()
    }
    
    data  # Return the modified data
  })
  
  
  output$interactivePlot <- renderPlotly({
    validate(
      need(!is.null(input$selectedFields) && length(input$selectedFields) > 0, "Please select at least one field of study")
    )
    
    data <- data_to_plot()
    p <- ggplot(data, aes(x = year, y = if(input$yAxisType == "percentage") { percentage } else { students }, group = major, color = major)) +
      geom_line(aes(text = paste("Year:", year, "<br>Major:", major, "<br>Students:", students))) +
      labs(title = "Student Data Over Years", x = "Year", y = if(input$yAxisType == "percentage") { "Percentage of Total Students" } else { "Number of Students" }) +
      theme_minimal() +
      theme(legend.position = "none")
    
    ggplotly(p, tooltip = "text")  # Using the 'text' aesthetic for tooltips
  })
  
  # Plot for student types
  output$studentTypePlot <- renderPlotly({
    data <- subset_by_year()$academic_detail_subset
    plot_ly(data, x = ~year, y = ~students, color = ~academic_level, type = 'bar', text = ~paste("Year:", year, "<br>Academic Level:", academic_level, "<br>Students:", students),
            hoverinfo = "text") %>%
      layout(yaxis = list(title = 'Number of Students'),
             barmode = 'stack',
             xaxis = list(title = 'Year'),
             title = 'Students Distribution by Academic Level Over Years')
  })
  
}

# Activate shinyApp
shinyApp(ui, server)
