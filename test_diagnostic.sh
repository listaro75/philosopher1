#!/bin/bash

# Test de diagnostic pour comprendre le problème du philosophe 2
echo "=== Test diagnostic 5 800 200 200 ==="
echo "Le philosophe 2 ne devrait PAS mourir à 1000ms"
echo

cd /home/luda-cun/42/Milestone_3/philosopher1

# Compilation
make > /dev/null 2>&1

echo "Exécution du test (timeout 15s)..."
timeout 15s ./philo 5 800 200 200 | tee /tmp/philo_diagnostic.log

echo
echo "=== Analyse du philosophe 2 ==="

# Suivre tous les événements du philosophe 2
echo "Timeline du philosophe 2 :"
grep "^[0-9]* 2 " /tmp/philo_diagnostic.log

echo
echo "=== Calcul théorique ==="
last_eating=$(grep "2 is eating" /tmp/philo_diagnostic.log | tail -1 | awk '{print $1}')
if [ -n "$last_eating" ]; then
    theoretical_death=$((last_eating + 800))
    echo "Dernier repas du philo 2 : ${last_eating}ms"
    echo "Mort théorique attendue : ${theoretical_death}ms"
else
    echo "Le philosophe 2 n'a jamais mangé - mort attendue à 800ms"
fi

# Trouver le moment de la mort
death_line=$(grep "2 died" /tmp/philo_diagnostic.log)
if [ -n "$death_line" ]; then
    death_time=$(echo "$death_line" | awk '{print $1}')
    echo "Mort réelle : ${death_time}ms"
    
    if [ -n "$last_eating" ]; then
        elapsed=$((death_time - last_eating))
        echo "Temps écoulé depuis le dernier repas : ${elapsed}ms"
        if [ "$elapsed" -gt 800 ]; then
            echo "✓ Mort justifiée (> 800ms)"
        else
            echo "✗ Mort prématurée (< 800ms) - PROBLÈME!"
        fi
    fi
else
    echo "Le philosophe 2 n'est pas mort"
fi

rm -f /tmp/philo_diagnostic.log
