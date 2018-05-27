visualize: visualizesemiorderlinearsearch.c 
	gcc -o visualize $(CFLAGS) visualizesemiorderlinearsearch.c
visualize2: visualizesemiordermultisearch.c
	gcc -o visualize2 visualizesemiordermultisearch.c -fopenmp
clean:
	rm visualize -rf
