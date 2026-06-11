data <- read.csv("~/Documents/ComPar/P03/Proyecto/datos_358.csv")

sse <- function(i)
{
  return (0.320116554 - 0.1817172839 * log10(i) - 0.6010809504 * i + 0.1190323989 * log(1 - 28.35719263 * i))
}

sae <- function(i)
{
  return (0.4379643846 - 0.1472349284 * log10(i) - 0.6504138376 * i + 0.1288694577 * log(1 - 29.32773612 * i))
}

mae <- function(i)
{
  return (0.3461434008 - 0.07304301488 * log(i) - 0.3570629758 * i + 0.1380816888 * log(1 - 25.92785564 * i))  
}

plot(x = data$corriente, y = data$voltaje, type = "l", ylim = c(0, 1.0), main = "Voltaje respecto a la corriente", ylab = "Voltaje", xlab = "Corriente")

x <- data$corriente
y <- sse(data$corriente)
lines(x = x, y = y, type = "l", col = "red")

y <- sae(data$corriente)
lines(x = x, y = y, type = "l", col = "blue")

y <- mae(data$corriente)
lines(x = x, y = y, type = "l", col = "green")
