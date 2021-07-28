all: compare

war_and_peace.txt:
	curl http://www.gutenberg.org/files/2600/2600-0.txt -o war_and_peace.txt -v -L

.PHONY: peace_and_war.txt
peace_and_war.txt: war_and_peace.txt
	./encode war_and_peace.txt 10000
	./decode peace_and_war.txt

.PHONY: compare
compare: peace_and_war.txt
	@echo
	diff -usq war_and_peace.txt peace_and_war.txt
