# Traduction

Merci de votre aide!

Comment puis-je aider à traduire QOwnNotes?

::: warning
Merci de ne pas essayer de modifier les fichiers de traductions dans `src/languages` directement. Ils sont tous générés automatiquement via [Crowdin](https://crowdin.com/project/qownnotes).
:::

## Rejoignez-nous sur Crowdin pour traduire QOwnNotes

![crowdin](/img/crowdin.png)

**Crowdin** est une plate-forme de gestion de traductions populaire pour les applications mobiles, le Web, les logiciels de bureau et les ressources associées.

Vous pouvez vous inscrire sur [Crowdin](https://crowdin.com/project/qownnotes) et **traduire** **QOwnNotes** dans votre langue avec des outils faciles à prendre en main.

**[QOwnNotes sur Crowdin](https://crowdin.com/project/qownnotes)**

Il suffit d’[envoyer un message au propriétaire du projet sur Crowdin](https://crowdin.com/profile/pbek) ou en tant qu’[Issue GitHub](https://github.com/pbek/QOwnNotes/issues) si vous souhaitez traduire QOwnNotes dans une nouvelle langue.

Si vous souhaitez ajouter de nouvelles traductions aux **langues existantes**, vous pouvez le faire immédiatement.

## Relecture

Si vous êtes un excellent locuteur ou même natif d'une langue déjà traduite qui n'a pas encore été relue, nous vous remercions de contribuer à rendre l'expérience QOwnNotes plus fiable et plus pratique en vérifiant et si nécessaire, en corrigeant les traductions non approuvées. Vous devez demander au propriétaire la permission d'être un relecteur pour votre langue.

Cliquez ici pour voir si votre langue fait partie des langues traduites !

[QOwnNotes sur Crowdin](https://translate.qownnotes.org/)

Une fois que vous avez [contacté le propriétaire du projet sur Crowdin](https://crowdin.com/profile/pbek) et obtenu l’autorisation de relire dans votre langue, vous êtes prêt à commencer !

## Conseils généraux

Veuillez prendre en compte quelques éléments lorsque vous travaillez sur le projet.

### Exactitude

Veuillez traduire avec une exactitude maximale tout en veillant à la fluidité dans votre langue cible.

### Précision

La cible doit représenter la source exactement et sans omission ni ajout d'aucune partie de la chaîne.

### Le contexte

Si vous avez besoin de plus de contexte ou d'aide avec une certaine chaîne et que vous connaissez une autre langue, vous pouvez changer la langue et voir ce que les autres traducteurs ont fait à partir du segment. Mais n'hésitez pas à laisser un message et à demander des éclaircissements sur le côté droit de la page.

### Termes clés fréquemment utilisés

Il est nécessaire de vérifier et d'utiliser les mêmes traductions pour les termes clés fréquemment utilisés, par exemple, « note » doit toujours être traduit de la même manière.

Si `trashed notes` est traduit par `notes supprimées` en français, il faudra toujours le traduire en utilisant cette expression. L'exception serait lorsque vous êtes grammaticalement obligé d'utiliser une alternative, mais restez simple.

![copie d'écran](/img/crowdin/screenshot-7.png)

Afin de trouver toutes les expressions identiques, recherchez dans Crowdin pour voir si toutes les occurrences d'un terme ont effectivement la même traduction. Si vous voulez proposer mieux, vous pouvez suggérer une meilleure traduction et voter pour la meilleure traduction / la traduction la plus correcte.

Traduisez toutes les occurrences de la manière la plus identique possible.

`Note` doit toujours avoir une seule traduction.

### Formes du pluriel

Pour le pluriel, veuillez ajouter la traduction appropriée dans le bloc correspondant.

![copie d'écran](/img/crowdin/screenshot-4.png)

### Variables

Les variables `%1` doivent être écrites sans espace. Dans les langages s’écrivant de droite à gauche, il est possible d'insérer les variables depuis l’original ou de les écrire dans votre traduction.

![copie d'écran](/img/crowdin/screenshot-1.png)

![copie d'écran](/img/crowdin/screenshot-5.png)

![copie d'écran](/img/crowdin/screenshot-3.png)

### Clés d'accès

Utilisez `&` devant toute lettre qui devrait être utilisée dans un raccourci. Elle peut être identique à l'originale mais ce n'est pas une obligation.

![copie d'écran](/img/crowdin/screenshot-4.png)

### Conteneurs personnalisés

Les chaînes telles que `::: tip`, `::: warning` et `::: info` dans la documentation sont des marqueurs pour des [conteneurs personnalisés](https://vuepress.vuejs.org/guide/markdown.html#custom-containers) dans VuePress et ne devraient pas être traduites.

### Autres chaînes à ne pas traduire

Quelques segments que nous ne traduisons pas dans la documentation :

- Le code informatique, les liens ou les étiquettes. Ils doivent être laissés en l'état et sans espaces ajoutés comme cela arrive avec certaines traductions automatiques. Ceux ajoutés doivent être effacés
- La date et l'heure (laissez vierge)
- Nous ne traduisons pas non plus les références que QOwnNotes possède dans de nombreuses langues traduites, vous pouvez les ignorer

## La traduction et la relecture peuvent être amusantes !

Vous devrez assimiler la terminologie de ce logiciel, apprendrez de nouveaux termes et recevrez de nombreuses informations internes à propos du projet.

Si vous souhaitez aider pour relire la traduction dans votre langue, contactez [Patrizio](https://crowdin.com/profile/pbek) pour poser vos questions et/ou obtenir les droits de relecture pour votre langue maternelle.

Merci beaucoup pour votre aide ! 🙂
