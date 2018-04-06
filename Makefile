visualize: visualizesemiorderlinearsearch.c 
	gcc -o visualize $(CFLAGS) visualizesemiorderlinearsearch.c

clean:
	rm visualize -rf
