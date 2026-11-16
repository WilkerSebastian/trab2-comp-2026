## USO_IA.md

Ferramenta: Qwen 3.8
Trecho: microc/microc.flex diretivas do %option
Finalidade: Entender o papel das diretivas %option prefix="microc_yy" e %option noyywrap exigidas pela integração do TP1 com o TP2, e por que elas são necessárias para o scanner.c fornecido conseguir linkar com o código gerado pelo flex.
O que fiz: Apenas consultado. A explicação foi usada para compreender as diretivas e aplicá-las corretamente no microc.flex.

Ferramenta: Qwen 3.8
Trecho: src/conjuntos.c alteração no conj_anulavel e criação atualizar_anulavel
Finalidade: Entender por que a recursão a esquerda indireta não era detectada sem o calculo de anulavel, e implementar conj_anulavel por ponto fixo, já que main.c o chama antes de verif_recursao_esquerda.
O que fiz: Apenas consultado. A explicação foi usada para compreender a dependencia.

Ferramenta: Qwen 3.8
Trecho: src/codegen.c gen_casamento_regex e funções auxiliares
Finalidade: Entender como converter cada expressão regular de TOKENS em um AFD.
O que fiz:  A explicação foi usada para compreender o pipeline de regex pra AFN pra AFD pra C.