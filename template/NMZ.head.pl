<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
        "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<!-- LINK-REV-MADE -->
<link rev="made" href="mailto:foobar@namazu.org">
<!-- LINK-REV-MADE -->
<title>Namazu: System Wyszukiwania Pe�notekstowego </title>
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
<h1>Namazu: System Wyszukiwania Pe�notekstowego</h1>
<p>
Ten indeks zawiera <!-- FILE --> 0 <!-- FILE --> dokument�w i
<!-- KEY --> 0 <!-- KEY --> s��w kluczowych.
</p>
<p>
<strong>Ostatnio zmodyfikowano: <!-- DATE --> date <!-- DATE --></strong>
</p>
<hr>
<form method="get" action="{cgi}">
<p>
<strong>Query:</strong>
<input type="text" name="query" size="40">
<input type="submit" value="Szukaj!">
<!-- <input type="hidden" name="idxname" value="foobar"> -->
<a href="{cgi}">[Jak szuka�, �eby znale�� a nie zab��dzi�]</a>
</p>
<p>
<strong>Pokazuj:</strong>
<select name="max">
<option value="10">10
<option selected value="20">20
<option value="30">30
<option value="50">50
<option value="100">100
</select>
<strong>Opis:</strong>
<select name="result">
<option selected value="normal">normalny
<option value="short">skr�cony
</select>
<strong>Kryterium sortowania:</strong>
<select name="sort">
<option selected value="score">wynik
<option value="date:late">data, od najnowszych
<option value="date:early">data, od najstarszych
<option value="field:subject:ascending">tytu�, w porz�dku rosn�cym
<option value="field:subject:descending">tytu�, w porz�dku malej�cym
<option value="field:from:ascending">autor, w porz�dku rosn�cym
<option value="field:from:descending">autor, w porz�dku malej�cym
<option value="field:size:ascending">rozmiar, w porz�dku rosn�cym
<option value="field:size:descending">rozmiar, w porz�dku malej�cym
<option value="field:uri:ascending">URI, w porz�dku rosn�cym
<option value="field:uri:descending">URI, w porz�dku malej�cym
</select>
</p>
<!--
<p>
<strong>Cel:</strong>
<ul>
<li><input type="checkbox" name="idxname" value="foo" checked>aptgept
<li><input type="checkbox" name="idxname" value="bar">fooshmoo
<li><input type="checkbox" name="idxname" value="baz">grepgzip
</ul>
</p>
-->
</form>
