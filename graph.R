data <- read.csv("~/Projects/ComPar/optimisaurio/datos_358.csv")

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
  return (0.3164392263 - 0.07807540317 * log(i) - 0.1333365113 * i + 0.1292480823 * log(1 - 27.61024852 * i))  
}

plot(x = data$corriente, y = data$voltaje, type = "l", ylim = c(0, 1.0))

x <- data$corriente
y <- sse(data$corriente)
lines(x = x, y = y, type = "l", col = "red")

y <- sae(data$corriente)
lines(x = x, y = y, type = "l", col = "blue")

y <- mae(data$corriente)
lines(x = x, y = y, type = "l", col = "green")
