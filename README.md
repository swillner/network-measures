# Network measures
Code to calculate network measures for multi-regional input-output (MRIO) tables as used on http://zeean.net/visualization


## Betweenness Centrality
The Betweenness Centrality reflects a country’s importance within the global supply network considering two factors: its connectivity to other regions and the amount of products (measured in US $) it exchanges with them. Concretely, it counts the number of shortest paths from all regional sectors to all other regional sectors that pass through that country. The size of the flows along those paths is not taken into account (either there is a flow > 1M$ or none).

Algorithm based on: Brandes, U. (2001). [A faster algorithm for betweenness centrality](http://www.tandfonline.com/doi/abs/10.1080/0022250x.2001.9990249). The Journal of Mathematical Sociology, 25(2), 163-177.


## Flow Centrality
The "Flow Centrality" we use here measures the number of maximum through-flow paths in the network of regional sectors passing through each of the regional sectors. A through-flow path between two regional sectors is a path connecting these two network nodes via flows associated with the minimum of all flows along that path. The through-flow paths associated with the maximal value of all these paths are maximum through-flow paths between those two nodes.
The Flow Centrality is thus similar to the Betweenness Centrality but unlike the latter takes the size of flows into account.

Algorithm is also based on: Brandes, U. (2001). [A faster algorithm for betweenness centrality](http://www.tandfonline.com/doi/abs/10.1080/0022250x.2001.9990249). The Journal of Mathematical Sociology, 25(2), 163-177.


## Global Adaptive Pressure
The Global Adaptive Pressure (GAP) reflects a country’s importance within the global supply network. It measures how dependent all other regions are on direct (1st order) and indirect (2nd order) supplies from this country.

The value of GAP provides the percentage production failure of the total production of a country due to the direct or indirect effects of a complete production failure in the selected country (1=100%). GAP is thereby an upper limit of the direct or indirect influence mainly because it assumes that the supply failure is not compensated by enhanced supply from other regions and because it assumes perfect complementarity of the production process.

More specific its definition is, for 0th order (let U be the set of forced regional sectors, e.g. in zeean all regional sectors in a chosen region):

![](http://www.sciweavers.org/tex2img.php?eq=%5Ctext%7BGAP%7D%5E%7B%280%29%7D_%7BU%5Crightarrow%20js%7D%20%5Cequiv%20%5Cbegin%7Bcases%7D1%20%26%20js%5Cin%20U%5C%5C0%20%26%20js%5Cnotin%20U%5Cend%7Bcases%7D&bc=Transparent&fc=Black&im=png&fs=12&ff=mathpazo&edit=0)
<!-- \text{GAP}^{(0)}_{U\rightarrow js} \equiv \begin{cases}1 & js\in U\\0 & js\notin U\end{cases} -->

Then the k-th order is defined recursively as:

![](http://www.sciweavers.org/tex2img.php?eq=%5Ctext%7BGAP%7D%5E%7B%28k%29%7D_%7BU%5Crightarrow%20js%7D%20%5Cequiv%201-%5Cmin_%7B%5Ctext%7BSectors%20%7Di%7D%5Cleft%28%5Cfrac%7B%5Csum_%7B%5Ctext%7BRegions%20%7Dr%7D%281-%5Ctext%7BGAP%7D%5E%7B%28k-1%29%7D_%7BU%5Crightarrow%20ir%7D%29%5Ccdot%20Z_%7Bir%5Crightarrow%20js%7D%7D%7B%5Csum_%7B%5Ctext%7BRegions%20%7Dr%7DZ_%7Bir%5Crightarrow%20js%7D%7D%5Cright%29&bc=Transparent&fc=Black&im=png&fs=12&ff=mathpazo&edit=0)
<!-- \text{GAP}^{(k)}_{U\rightarrow js} \equiv 1-\min_{\text{Sectors }i}\left(\frac{\sum_{\text{Regions }r}(1-\text{GAP}^{(k-1)}_{U\rightarrow ir})\cdot Z_{ir\rightarrow js}}{\sum_{\text{Regions }r}Z_{ir\rightarrow js}}\right) -->

And for a "target" region r:

![](http://www.sciweavers.org/tex2img.php?eq=%5Ctext%7BGAP%7D%5E%7B%28k%29%7D_%7BU%5Crightarrow%20r%7D%20%5Cequiv%20%5Csum_%7B%5Ctext%7BSectors%20%7Di%7D%5Cfrac%7B%281-%5Ctext%7BGAP%7D%5E%7B%28k-1%29%7DZ_%7BU%5Crightarrow%20ir%7D%29%5CcdotZ_%7Bir%5Crightarrow%20js%7D%7D%7B%5Csum_%7B%5Ctext%7BSectors%20%7Di%2C%5Ctext%7BRegional%20sectors%20%7Djs%7DZ_%7Bir%5Crightarrow%20js%7D%7D&bc=Transparent&fc=Black&im=png&fs=12&ff=mathpazo&edit=0)
<!-- \text{GAP}^{(k)}_{U\rightarrow r} \equiv \sum_{\text{Sectors }i}\frac{(1-\text{GAP}^{(k-1)}Z_{U\rightarrow ir})\cdotZ_{ir\rightarrow js}}{\sum_{\text{Sectors }i,\text{Regional sectors }js}Z_{ir\rightarrow js}} -->