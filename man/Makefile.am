man_MANS = namazu.1 mknmz.1

update-man:
	LANG=C help2man -N ../src/namazu > $@
	LANG=C help2man -N ../src/mknmz > $@

clean:
	rm $(man_MANS)