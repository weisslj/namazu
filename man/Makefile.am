man_MANS = namazu.1 mknmz.1

update-man: namazu.1 mknmz.1

namazu.1: ../src/namazu
	LANG=C help2man -N ../src/namazu > $@

mknmz.1: ../src/mknmz
	LANG=C help2man -N ../src/mknmz > $@

clean:
	rm $(man_MANS)