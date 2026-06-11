camel <- read.csv("~/Documents/ComPar/P03/Proyecto/times/1p/camel.out")
sinoidal <- read.csv("~/Documents/ComPar/P03/Proyecto/times/1p/sinoidal.out")
rosenbrock <- read.csv("~/Documents/ComPar/P03/Proyecto/times/1p/rosenbrock.out")
michaelwicz <- read.csv("~/Documents/ComPar/P03/Proyecto/times/1p/michaelwicz.out")

times1p <- data.frame(Time = seq(1, 25))
times1p$camel <- camel$time
times1p$sinoidal <-sinoidal$time
times1p$rosenbrock <-rosenbrock$time
times1p$michaelwicz <- michaelwicz$time

camel <- read.csv("~/Documents/ComPar/P03/Proyecto/times/2p/camel.out")
sinoidal <- read.csv("~/Documents/ComPar/P03/Proyecto/times/2p/sinoidal.out")
rosenbrock <- read.csv("~/Documents/ComPar/P03/Proyecto/times/2p/rosenbrock.out")
michaelwicz <- read.csv("~/Documents/ComPar/P03/Proyecto/times/2p/michaelwicz.out")

times2p <- data.frame(Time = seq(1, 25))
times2p$camel <- camel$time
times2p$sinoidal <-sinoidal$time
times2p$rosenbrock <-rosenbrock$time
times2p$michaelwicz <- michaelwicz$time

camel <- read.csv("~/Documents/ComPar/P03/Proyecto/times/4p/camel.out")
sinoidal <- read.csv("~/Documents/ComPar/P03/Proyecto/times/4p/sinoidal.out")
rosenbrock <- read.csv("~/Documents/ComPar/P03/Proyecto/times/4p/rosenbrock.out")
michaelwicz <- read.csv("~/Documents/ComPar/P03/Proyecto/times/4p/michaelwicz.out")

times4p <- data.frame(Time = seq(1, 25))
times4p$camel <- camel$time
times4p$sinoidal <-sinoidal$time
times4p$rosenbrock <-rosenbrock$time
times4p$michaelwicz <- michaelwicz$time

camel <- read.csv("~/Documents/ComPar/P03/Proyecto/times/6p/camel.out")
sinoidal <- read.csv("~/Documents/ComPar/P03/Proyecto/times/6p/sinoidal.out")
rosenbrock <- read.csv("~/Documents/ComPar/P03/Proyecto/times/6p/rosenbrock.out")
michaelwicz <- read.csv("~/Documents/ComPar/P03/Proyecto/times/6p/michaelwicz.out")

times6p <- data.frame(Time = seq(1, 25))
times6p$camel <- camel$time
times6p$sinoidal <-sinoidal$time
times6p$rosenbrock <-rosenbrock$time
times6p$michaelwicz <- michaelwicz$time

avg1p <- data.frame(tcamel = mean(times1p$camel),
                    tsinoidal = mean(times1p$sinoidal),
                    trosenbrock = mean(times1p$rosenbrock),
                    tmichaelwicz = mean(times1p$michaelwicz))

avg2p <- data.frame(tcamel = mean(times2p$camel),
                    tsinoidal = mean(times2p$sinoidal),
                    trosenbrock = mean(times2p$rosenbrock),
                    tmichaelwicz = mean(times2p$michaelwicz))

avg4p <- data.frame(tcamel = mean(times4p$camel),
                    tsinoidal = mean(times4p$sinoidal),
                    trosenbrock = mean(times4p$rosenbrock),
                    tmichaelwicz = mean(times4p$michaelwicz))

avg6p <- data.frame(tcamel = mean(times6p$camel),
                    tsinoidal = mean(times6p$sinoidal),
                    trosenbrock = mean(times6p$rosenbrock),
                    tmichaelwicz = mean(times6p$michaelwicz))

runtime_table <- rbind(avg1p, avg2p, avg4p, avg6p)
cores <- c(1, 2, 4, 6)

plot(x = cores,
     y = runtime_table$tcamel,
     type = "l",
     col = "red",
     ylim = c(0, 4000),
     xlab = "Hilos",
     ylab = "ms",
     main = "Tiempos de ejecución (ms)"
)

lines(x = cores,
      y = runtime_table$tsinoidal,
      type = "l",
      col = "blue",
      )

lines(x = cores,
      y = runtime_table$trosenbrock,
      type = "l",
      col = "green",
)

lines(x = cores,
      y = runtime_table$tmichaelwicz,
      type = "l",
      col = "orange",
      )
grid(nx = NULL, ny = NULL, col = "lightgray", lty = "dotted")

camel1p <- runtime_table$tcamel[1]
sinoidal1p <- runtime_table$tsinoidal[1]
rosenbrock1p <- runtime_table$trosenbrock[1]
michaelwicz1p <- runtime_table$tmichaelwicz[1]

camel_ef <- camel1p / runtime_table$tcamel
sinoidal_ef <- sinoidal1p / runtime_table$tsinoidal
rosenbrock_ef <- rosenbrock1p / runtime_table$trosenbrock
michaelwicz_ef <- michaelwicz1p / runtime_table$tmichaelwicz

plot(x = cores,
     y = camel_su,
     type = "l",
     col = "red",
     ylim = c(1.0, 6.0),
     ylab = "Speedup relativo",
     xlab = "Hilos",
     main = "Speedup"
     )
lines(x = seq(1, 6),
      y = seq(1, 6),
      lty = "dotted"
      )

lines(x = cores,
      y = sinoidal_su,
      type = "l",
      col = "blue"
      )

lines(x = cores,
      y = rosenbrock_su,
      type = "l",
      col = "green"
      )
lines(x = cores, 
      y = michaelwicz_su,
      type = "l",
      col = "orange")
grid(nx = NULL, ny = NULL, col = "lightgray", lty = "dotted")

camel_ef = camel_ef / cores
sinoidal_ef = sinoidal_ef / cores
rosenbrock_ef = rosenbrock_ef / cores
michaelwicz_ef = michaelwicz_ef / cores

plot(x = cores,
     y = camel_ef,
     type = "l",
     col = "red",
     ylim = c(0.5, 1.5),
     ylab = "Eficiencia",
     xlab = "Hilos",
     main = "Eficiencia"
)


lines(x = cores,
      y = sinoidal_ef,
      type = "l",
      col = "blue"
)

lines(x = cores,
      y = rosenbrock_ef,
      type = "l",
      col = "green"
)
lines(x = cores, 
      y = michaelwicz_ef,
      type = "l",
      col = "orange")
grid(nx = NULL, ny = NULL, col = "lightgray", lty = "dotted")