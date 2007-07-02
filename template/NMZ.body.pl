<h2><a name="query" id="query">Zapytania</a></h2>

<h3><a name="query-term" id="query-term">Zapytanie pojedyńcze</a></h3>
<p>
To zapytanie wyszczególnia tylko jedno określenie dla wyszukiwania wszystkich
dokumentów, które zawierają to określenie, np.:
</p>

<p class="example">
namazu
</p>

<h3><a name="query-and" id="query-and">Zapytanie typu AND(logiczne i)</a></h3>

<p>
To zapytanie wyszczególnia 2 lub więcej określeń dla wyszukiwania wszystkich
dokumentów, które zawierają wszystkie podane określenia. Możesz wpisać operator
<code class="operator">and</code> pomiędzy dwoma lub więcej określeniami, np.:
</p>

<p class="example">
Linux and Netscape
</p>

<p>
Możesz pominąć operator
<code class="operator">and</code> .
Określenia, które są oddzielone jedną lub więcej spacją są uważane za zapytanie typu AND.
</p>

<h3><a name="query-or" id="query-or">Zapytanie typu OR(logiczne lub)</a></h3>
<p>
To zapytanie wyszczególnia 2 lub więcej określeń dla wyszukiwania wszystkich
dokumentów, które zawierają jakiekolwiek z podanych określeń. Możesz wpisać operator
<code class="operator">or</code> pomiędzy dwoma lub więcej określeniami, np.:
</p>

<p class="example">
Linux or FreeBSD
</p>

<h3><a name="query-not" id="query-not">Zapytanie typu NOT(przeczenie)</a></h3>
<p>
To zapytanie wyszczególnia 2 lub więcej określeń dla wyszukiwania wszystkich
dokumentów, które zawierają pierwsze określenie, ale nie zawierają
następnych określeń. Możesz wpisać operator <code class="operator">not</code>
 pomiędzy dwoma lub więcej określeniami, np.:
</p>

<p class="example">
Linux not UNIX
</p>


<h3><a name="query-grouping" id="query-grouping">Grupowanie</a></h3>
<p>
Możesz grupować zapytania przez zawarcie ich
w nawiasach okragłych. Nawiasy powinny być oddzielone przez jedną lub
więcej spacji, np.:
</p>

<p class="example">
( Linux or FreeBSD ) and Netscape not Windows
</p>

<h3><a name="query-phrase" id="query-phrase">Wyszukiwanie fraz</a></h3>
<p>
Możesz szukać wyrażenia, które składa się z dwóch lub więcej określeń
otaczając je podwójnym cudzysłowem jak
<code class="operator">"..."</code> lub klamrami jak <code class="operator">{...}</code>.
Precyzja Namazu  w wyszukiwaniu fraz nie jest 100%,
lecz błędne wyniki trafiają się bardzo rzadko, np.:
</p>

<p class="example">
{GNU Emacs}
</p>

<!-- foo
<p>
You must choose the latter with Tkanamzu or namazu.el.
Musisz wybrać ten ostatni z  Tkanamzu lub namazu.el. ???
</p>
-->

<h3><a name="query-substring" id="query-substring">Wyszukiwanie okrojonych słów</a></h3>
<p>
Są trzy typy wyszukiwania okrojonych słów.
</p>

<dl>
<dt>Przedrostkowe dopasowanie</dt>
<dd><code class="example">inter*</code> (określenia rozpoczynające się na <code>inter</code>)</dd>
<dt>Wewnętrzne dopasowanie</dt>
<dd><code class="example">*text*</code> (określenia zawierające się wewnątrz <code>text</code>)</dd>
<dt>Przyrostkowe dopasowanie</dt>
<dd><code class="example">*net</code> (określenia kończące się na <code>net</code>)</dd>
</dl>


<h3><a name="query-regex" id="query-regex">Wyrażenia regularne (man grep)</a></h3>

<p>
Możesz używać wyrażeń regularnych jako wzór do dopasowywania.
Wyrażenia regularne muszą być otoczone przez ukośniki tak, jak
<code
class="operator">/.../</code>. Namazu wykorzystuje silnik wyrażeń regularnych
<ahref="http://www.ruby-lang.org/">Ruby</a>. Ogólnie jest on kompatybilny z <a
href="http://www.perl.com/">Perl</a> ,
np.:
</p>

<p class="example">
/pro(gram|blem)s?/
</p>


<h3><a name="query-field" id="query-field">Wyszukiwanie po wyspecyfikowanych polach</a></h3>
<p>
Możesz ograniczyć przeszukiwanie do określonych pól jak
<code>Subject:</code>, <code>From:</code>,
<code>Message-Id:</code>. Jest to zwłaszcza dogodne dla dokumentów typu
Mail/News, np.:
</p>

<ul>
<li><code class="example">+subject:Linux</code><br>
(Zwraca wszystkie dokument zawierające pola <code>Linux</code>
w <code>Subject:</code> )
</li>

<li><code class="example">+subject:"GNU Emacs"</code><br>
(Zwraca wszystkie dokumenty zawierające pola <code>GNU Emacs</code>
w <code>Subject:</code> )
</li>

<li><code class="example">+from:foo@example.jp</code><br>
(Zwraca wszystkie dokument zawierające pola <code>foo@example.jp</code>
w <code>From:</code> )
</li>


<li><code class="example">+message-id:&lt;199801240555.OAA18737@foo.example.jp&gt;</code><br>
(Zwraca pewien dokument zawierający wyszczególnione
<code>Message-Id:</code>)
</li>
</ul>

<h3><a name="query-notes" id="query-notes">Uwagi</a></h3>

<ul>
<li>We wszystkich zapytaniach, Namazu ignoruje wielkość znaków alfabetu.
</li>

<li>Japanese phrases are forced to be segmented into
morphemes automatically and are handled them as <a
href="#query-phrase">phrase searching</a>. This processing
causes invalid segmentation occasionally.
<br>Po cholere to tłumaczyli na angielski? Ja. w każdym bądź razie nie będę, bo nie zamierzam posługiwać
się jęz. japońskim.
</li>

<li>Alphabet, numbers or a part of symbols (duplicated in
ASCII) characters which defined in JIS X 0208 (Japanese
Industrial Standards) are handled as ASCII characters.
<br>Już mam skośne oczy według normy przemysłowej JIS X 0208.
</li>

<li>Namazu radzi sobie z określeniami zawierającymi znaki takie jak
<code>TCP/IP</code>, ale to radzenie sobie nie jest kompletne
możesz opisać
<code>TCP and IP</code> zamiast
<code>TCP/IP</code>, ale może to być powodem zbyt wielu dopasowań (szumu informacyjnego)
</li>

<li>Dopasowania okrojonych słów i szukania po wybranych polach zabierają
więcej czasu, niż inne metody.
</li>

<li>Jeżeli chcesz używać <code class="operator">and</code>,
<code class="operator">or</code> lub <code
class="operator">not</code> po prostu jako określeń, możesz
otoczyć je odpowiednio podwójnym cudzysłowem tak, jak
<code
class="operator">"..."</code> lub klamrami tak, jak <code
class="operator">{...}</code>.

<!-- foo
You must choose the latter with Tkanamzu or namazu.el.
???????????????????????????????????? ho ku jank siu?
ja nie znaju sztoo eto.
-->
</li>

</ul>

