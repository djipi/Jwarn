#
#	makefile for jwarn
# 
CFLAGS = -O3 -c -std=c++14

OBJS = jwarn.o table.o rules.o

jwarn:	$(OBJS)
	g++ -o $@ $(OBJS)

jwarn.o: JWARN.C TABLE.H
	g++ $(CFLAGS) -o $@ $<

table.o: TABLE.C TABLE.H
	g++ $(CFLAGS) -o $@ $<

rules.o: RULES.C TABLE.H MESSAGE.H
	g++ $(CFLAGS) -o $@ $<

all: jwarn

clean:
	rm -f $(OBJS) jwarn
