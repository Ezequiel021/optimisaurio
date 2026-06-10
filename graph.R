data <- read.csv("~/Documents/ComPar/P03/Proyecto/datos_358.csv")

sse <- function(i)
{
  return (0.482723 - 0.05733 * log(i) - i * 0.891235 + 0.153874 * log(1 - 28.2386 * i))
}

sae <- function(i)
{
  return (0.500000 - 0.045037 * log(i) - i * 0.204889 + 0.232755 * log(1 - 24.8324 * i))
}

mae <- function(i)
{
  return (0.460381 - 0.110310 * log(i) - i * 0.548958 + 0.134323 * log(1 - 28.5105 * i))  
}

plot(x = data$corriente, y = data$voltaje, type = "l", ylim = c(0, 1.0))

x <- data$corriente
y <- sse(data$corriente)
lines(x = x, y = y, type = "l", col = "red")

y <- sae(data$corriente)
lines(x = x, y = y, type = "l", col = "blue")

y <- mae(data$corriente)
lines(x = x, y = y, type = "l", col = "green")
