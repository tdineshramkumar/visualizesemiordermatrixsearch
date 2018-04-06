all: visualize

visualize: visualizesemiorderlinearsearch.c
	gcc -o visualize visualizesemiorderlinearsearch.c

clean:
	rm visualize -rf
