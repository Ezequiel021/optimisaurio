data <- read.csv("~/Documents/ComPar/P03/Proyecto/results.csv")

summary(data$Generation)
summary(data$Solution)

plot(x = data$Run, y = data$Generation, type = "l")

plot(x = data$Run, y = data$Solution, type = "l")
x <- seq(100)
y <- rep(3.14159265, 100)
lines(x = x, y = y, type = "l", col = "red")
