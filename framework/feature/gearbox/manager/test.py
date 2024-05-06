import networkx as nx

# 创建一个无向图
G = nx.Graph()

# 添加一些节点和边
G.add_edges_from([(1, 2), (2, 3), (3, 4), (4, 5)])

print(str(G.edges()))
# 遍历输出每条边的信息
for u, v in G.edges():
    print(f"Edge ({u}, {v})")
