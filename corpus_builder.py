import feedparser

# change these
output_path = "corpora/arxiv"
categories = ["cs.AI", "q-bio.BM", "physics.optics", "astro-ph.GA"]
abstracts_per_category = [ 200, 250, 400, 800, 1000 ]

def fetch_arxiv_abstracts(categories, abstracts_per_category):
    entries = []
    for category in categories:
        url = (
            "http://export.arxiv.org/api/query"
            f"?search_query=cat:{category}"
            f"&start=0&max_results={abstracts_per_category}"
        )
        feed = feedparser.parse(url)
        for item in feed.entries:
            # remove newlines
            abstract = item.summary.replace('\n', ' ').strip()
            label = item.tags[0]['term'] if hasattr(item, 'tags') else category
            entries.append((label, abstract))
    return entries

for count in abstracts_per_category:
    data = fetch_arxiv_abstracts(categories, count)
    path = f"{output_path}/raw_{count}.txt"
    with open(path, "w", encoding="utf-8") as file:
        for label, abstract in data:
            file.write(f"{label}:{abstract}\n")
    print(f"wrote {count} abstracts to '{path}'")
