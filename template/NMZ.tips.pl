<h2><a name="tips" id="tips">Porady dotyczące wyszukiwania</a></h2>

<p>
Jeżeli masz problem ze znalezieniem podanej przez Ciebie informacji, spróbuj zastosować poniższe rady
</p>

<ul>
<li>Sprawdź pisownię podanych słów kluczowych<br>
Namazu nic nie znajdzie z błędami w pisowni.
</li>

<li>Dodaj słów-kluczy<br>

Jeżeli nie otrzymujesz wyników, lub otrzymujesz ich za mało, możesz dodać jedno
lub więcej pokrewnych słów z operatorem
<code
class="operator">or</code> . Powinieneś otrzymać więcej rezultatów np.:
<br>
<code class="example">tex or ptex or latex or latex2e</code><br>

Jezeli otrzymujesz za dużo wynikow, możesz dodać jedno lub więcej
pokrewnych słów z operatorem
<code class="operator">and</code>
. To ograniczy bardziej zakres przeszukiwania np.:<br>
<code class="example">latex and dvi2ps and eps</code>
</li>

<li>Spróbuj wyszukiwania okrojonych słów<br>

Jeżeli nie otrzymujesz wyników, lub otrzymujesz ich za mało, możesz spróbować
dopasowywania okrojonych słów.
Możesz wyszczególnić<code class="example">tex*</code> dla wyszukania określeń zaczynających się na
<code>tex</code> (e.g., <code>tex</code>,
<code>texi2html</code>,
<code>texindex</code>, <code>text</code>).
<br>
Możesz wyszczególnić<code class="example">*tex</code> to
wyszukiwanie dla określeń kończących się na
<code>tex</code> (np.:
<code>bibtex</code>,
<code>jlatex</code>, <code>latex</code>,
<code>platex</code>, <code>ptex</code>, <code>vertex</code>).
<br>

Możesz wyszczególnić
<code class="example">*tex*</code> to
wyszukiwanie dla określeń zawierających
<code>tex</code> (wiele).
<br>
</li>

<li>You tried phrase searching but it hit documents which
Spróbowałeś wyszukiwania fraz, ale to nie zwróciło żadnych wyników zawierających twoją frazę?
<br>
To jest usterka Namazu. Precyzja wyszukiwania fraz nie jest
100%, ale złe wyniki są rzadkie.
</li>

<li>Jeżeli chcesz użyć <code class="operator">and</code>,
<code class="operator">or</code> lub <code
class="operator">not</code> jako zwykłych słów kluczowych <br>
możesz otoczyć je odpowiednio podwójnymi cudzysłowami tak, jak
<code
class="operator">"..."</code> lub takimi nawiasami <code
class="operator">{...}</code>.
</li>

</ul>
