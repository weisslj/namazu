<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
        "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<!-- LINK-REV-MADE -->
<link rev="made" href="mailto:foobar@namazu.org">
<!-- LINK-REV-MADE -->
<title>Namazu: System Wyszukiwania Pełnotekstowego </title>
<style type="text/css"><!--
  strong.keyword { color: Red; }
  p.example      { text-indent: 1em;
                   color: Navy;
		   font-weight: bold;
                   font-family: monospace; }
  code           { color: Navy;
                   font-family: monospace; }
  code.example   { color: Navy;
		   font-weight: bold;
                   font-family: monospace; }
  code.operator  { color: Navy;
                   font-family: monospace;
		   font-weight: bold; }
--></style>
</head>
<body lang="pl">
<h1>Namazu: System Wyszukiwania Pełnotekstowego</h1>
<p>
Ten indeks zawiera <!-- FILE --> 0 <!-- FILE --> dokumentów i
<!-- KEY --> 0 <!-- KEY --> słów kluczowych.
</p>
<p>
<strong>Ostatnio zmodyfikowano: <!-- DATE --> date <!-- DATE --></strong>
</p>
<hr>
<form method="get" action="{cgi}">
<p>
<strong>Query:</strong>
<input type="text" name="query" size="40" value="">
<input type="submit" name="submit" value="Szukaj!">
<!-- <input type="hidden" name="idxname" value="foobar"> -->
<a href="{cgi}">[Jak szukać, żeby znaleźć a nie zabłądzić]</a>
</p>
<p>
<strong>Pokazuj:</strong>
<select name="max">
<option value="10">10</option>
<option selected value="20">20</option>
<option value="30">30</option>
<option value="50">50</option>
<option value="100">100</option>
</select>
<strong>Opis:</strong>
<select name="result">
<option selected value="normal">normalny</option>
<option value="short">skrócony</option>
</select>
<strong>Kryterium sortowania:</strong>
<select name="sort">
<option selected value="score">wynik</option>
<option value="date:late">data, od najnowszych</option>
<option value="date:early">data, od najstarszych</option>
<option value="field:subject:ascending">tytuł, w porządku rosnącym</option>
<option value="field:subject:descending">tytuł, w porządku malejącym</option>
<option value="field:from:ascending">autor, w porządku rosnącym</option>
<option value="field:from:descending">autor, w porządku malejącym</option>
<option value="field:size:ascending">rozmiar, w porządku rosnącym</option>
<option value="field:size:descending">rozmiar, w porządku malejącym</option>
<option value="field:uri:ascending">URI, w porządku rosnącym</option>
<option value="field:uri:descending">URI, w porządku malejącym</option>
</select>
</p>
<!--
<p>
<strong>Cel:</strong>
<ul>
<li><input type="checkbox" name="idxname" value="foo" checked>aptgept</li>
<li><input type="checkbox" name="idxname" value="bar">fooshmoo</li>
<li><input type="checkbox" name="idxname" value="baz">grepgzip</li>
</ul>
</p>
-->
</form>
