.PHONY: tweets snakes clean all

CC = gcc
CCFLAGS = -Wall -Wextra -Wvla
EXTRA = markov_chain.o linked_list.o
TWEETS = tweets_generator.c $(EXTRA)
SNAKES = snakes_and_ladders.c $(EXTRA)

all: tweets snakes

tweets: $(TWEETS)
	$(CC) $^ -o tweets_generator

snakes: $(SNAKES)
	$(CC) $^ -o snakes_and_ladders

markov_chain.o: markov_chain.c markov_chain.h
	$(CC) $(CCFLAGS) -c $^

linked_list.o: linked_list.c linked_list.h
	$(CC) $(CCFLAGS) -c $^

tweets_generator.o: tweets_generator.c
	$(CC) $(CCFLAGS) -c $^

snakes_and_ladders.o: snakes_and_ladders.c
	$(CC) $(CCFLAGS) -c $^



clean:
	rm -f *.o *.gch tweets_generator snakes_and_ladders