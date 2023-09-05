# read from csv to df
df <- read.csv("test.csv")
# print df
print(df)
# get minimum in second column
min <- min(df[,2])
# plot second and third column as function of first
plot(df[,1], df[,2], xlab="x", ylab="y")
# perform anova on second and third column
anova(df[,2], df[,3])
