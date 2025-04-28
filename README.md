# Projeto Multithread - MC504

O projeto aborda o problema 6.3 - *"Baboon crossing problem"* do livro *The Little Book of Semaphores*. O problema consiste em um precípio com uma única corda que conecta os dois lados do canyon, com babuínos em ambos os lados querendo atravessar. Assim, temos algumas limitações:

- A corda aguenta até 5 babuínos ao mesmo tempo
- Babuínos atravessando a corda em direções opostas não podem se encontrar na corda
- Os babuínos de um lado eventualmente devem conseguir atravessar para o outro lado *(starvation)*

O arquivo [Report](w1/src_latex/Report.pdf) contêm informações mais detalhadas sobre o problema, a solução encontrada pelo grupo e a explicação do código escrito.