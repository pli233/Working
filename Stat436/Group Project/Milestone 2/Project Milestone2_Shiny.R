#1. Include all required library

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
# uncomment below line to install d3treeR
# devtools::install_github("timelyportfolio/d3treeR") 


#2. Create Function to preprocess datasets by separating 'year' column
preprocess_dataset <- function(data) {
  filtered_data <- data %>% separate(year, sep = "/", into = "year", convert = TRUE, extra = "drop")
  return(filtered_data)
}


#3. Data Preprocess


#3.1 source of fund
source_of_fund <- preprocess_dataset(read_csv("data/source_of_fund.csv"))


#3.2 field_of_study
field_of_study <- read_csv("data/field_of_study.csv")
fos_tree <- preprocess_dataset(field_of_study)


#3.3 map_data
origin <- preprocess_dataset(read_csv("data/origin.csv"))


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
  #process data
  filtered_df <- df %>%
    filter(selected) %>%
    group_by(source_of_fund) %>%
    summarize(students = sum(students)) %>%
    mutate(percentage = round(students / sum(students) * 100, 1),
           label = ifelse(percentage > 5, paste0(percentage, "%"), NA))  # Only label slices >5% to avoid overlapping
  
  #plot pie graph
  ggplot(filtered_df, aes(x = 1, y = students, fill = source_of_fund)) +
    geom_bar(width = 1, stat = "identity") +
    coord_polar(theta = "y") +
    theme_void() +
    geom_text(aes(label = label), position = position_stack(vjust = 0.5), color = "black", size = 3.5) +
    scale_fill_brewer(palette = "Set3") +
    labs(fill = "Source of Fund", y = "Students", x = "") +
    theme(legend.position = "right",
          legend.direction = "vertical",
          legend.title = element_text(size = 12),
          legend.text = element_text(size = 10))
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

#4.3 tree graph generator
tree <- function(df) {
  filtered_df <- df %>% filter(selected)
  
  treemap(filtered_df,
          index = c("field_of_study", "major"),
          vSize = "students",
          type = "index",
          palette = "Set2",
          align.labels = list(c("center", "center"), c("right", "bottom")))
}

#4.4 inter graph generator
inter <- function(p) {
  d3tree2(p, rootname = "Field of Study")
}



#5. Shiny Application Logic

#5.1 retrieve the min and max year among two datasets
all_years <- c(source_of_fund$year, fos_tree$year)
min_year <- min(all_years)
max_year <- max(all_years)

#5.2 UI Logics

#5.2.1 create the header and assign title to it
header <- dashboardHeader(
  title = "International Education in the US"
)

#5.2.2 create the side bar with the options to choose graph
sidebar <- dashboardSidebar(
  sidebarMenu(
    menuItem("Filter Options", tabName = "filters", icon = icon("filter"), startExpanded = TRUE,
             sliderInput("yearRange", "Year of Release:",
                         min = min_year,
                         max = max_year,
                         value = c(min_year, max_year),
                         step = 1,
                         sep = "",
                         ticks = FALSE, # Removes tick marks for a cleaner look
                         animate = TRUE) # Adds animation to the slider for better user experience
    ),
    menuItem("Source of Fund", tabName = "fundSource", icon = icon("money-check-dollar")),
    menuItem("Student Origin", tabName = "studentOrigin", icon = icon("globe")),
    menuItem("Field of Study", tabName = "fieldOfStudy", icon = icon("book"))
  )
)

#5.2.3 create the dash board
body <- dashboardBody(
  tabItems(
    # Filter Options Tab
    tabItem(tabName = "filters", 
            h3("Use the slider to select the year range for your analysis."),
            tags$hr()
    ),
    # Source of Fund Tab
    tabItem(tabName = "fundSource", 
            plotOutput("pie")
    ),
    # Student Origin Tab
    tabItem(tabName = "studentOrigin", 
            plotOutput("map")
    ),
    # Field of Study Tab
    tabItem(tabName = "fieldOfStudy", 
            uiOutput("inter")
    )
  )
)
#5.2.4 assemble all ui logics to ui element
ui <- dashboardPage(header, sidebar, body)


#5.3 Server Logics
server <- function(input, output) {
  
  #5.3.1 Reactive expression for filtering datasets based on yearRange
  subset_by_year <- reactive({
    list(
      fund_subset = source_of_fund %>%
        mutate(selected = (year >= input$yearRange[1]) & (year <= input$yearRange[2])),
      test_subset = test %>%
        mutate(selected = (year >= input$yearRange[1]) & (year <= input$yearRange[2])),
      tree_subset = field_of_study %>%
        mutate(selected = (year >= input$yearRange[1]) & (year <= input$yearRange[2]))
    )
  })
  
  #5.3.2 Use the reactive list elements for plotting and UI rendering
  output$pie <- renderPlot({
    pie(subset_by_year()$fund_subset)
  })
  
  output$map <- renderPlot({
    map(subset_by_year()$test_subset)
  })
  
  output$inter <- renderUI({
    inter(tree(subset_by_year()$tree_subset))
  })
  
}

#5.3 Activate shinyapp
shinyApp(ui, server)






