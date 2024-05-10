import subprocess
import json
import networkx as nx
import matplotlib.pyplot as plt

def run_bpftool(command):
    result = subprocess.run(["bpftool", command, "show", "-j"], capture_output=True, text=True)
    return json.loads(result.stdout)

def filter_programs_with_type(programs, prog_type):
    filtered = []
    for prog in programs:
        for _type in prog_type:
            if prog['type'] == _type:
                filtered.append(prog)
    return filtered

def find_links_for_programs(programs, links):
    prog_ids = {prog['id'] for prog in programs}
    return [link for link in links if link['prog_id'] in prog_ids]

def visualize_links(links, filename):
    G = nx.DiGraph()
    for link in links:
        G.add_edge(link['id'], link['prog_id'])
    pos = nx.spring_layout(G)
    nx.draw(G, pos, with_labels=True, node_color='lightblue', edge_color='gray', node_size=2000, font_size=9)
    plt.savefig(filename)
    plt.close()

def save_to_file(progs, maps, links, filename):
    with open(filename, 'w') as file:
        json.dump({'programs': progs, 'maps': maps, 'links': links}, file, indent=4)

def main():
    # 获取数据
    progs = run_bpftool("prog")
    maps = run_bpftool("map")
    links = run_bpftool("link")

    # 筛选有特定comm的程序
    filter_comm = "loader-capsule"
    filtered_progs = filter_programs_with_type(progs, ['tracepoint', 'sockops'])

    # 找到这些程序的链接
    relevant_links = find_links_for_programs(filtered_progs, links)

    # 可视化链接并保存到文件
    visualize_links(relevant_links, './output/bpf_status.png')

    # 保存到文本到文件
    save_to_file(filtered_progs, maps, links, "./output/bpf_status.json")

if __name__ == "__main__":
    main()
