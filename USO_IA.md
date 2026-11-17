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

Ferramenta: Qwen 3.8
Trecho: src/ll1.c no verificar_opc_rep e microc/microc.flex regra EOF
Finalidade: Diagnosticar duas falhas que apareceram ao rodar make test, a primeira sendo mensagem do aviso LL(1) estava incompleta, faltava o trecho" (resolvido de forma gulosa)", exigido pelo arquivo esperado testes/esperado/microc.gerador, fazendo o teste gramatica microc. O outro era flex emitia tres avisos "varias regras EOF para a condicao de inicio COMMENT/CHAR_STATE/STR_STATE", porque a regra EOF sem prefixo de estado conflitava com as regras especificas desses estados.
O que fiz: Ajustei a string em reportar() dentro de verificar_opc_rep e restringir a regra EOF para INITIAL/EOF.

Ferramenta: Qwen 4.0
Trecho: Relatorio
Finalidade: Gerar a base LaTeX do relatório em formatado, com uma página por seção, e ajustar o formato conforme as revisões pedidas.
O que fiz: Gerou o código LaTeX e o texto inicial de cada seção a partir do que eu relatei sobre o desenvolvimento. Eu revisei o conteúdo para corresponder ao estado real do trabalho, ajustei o texto e corrigi os erros de compilação.