#!/bin/bash

# Test spécifique pour le problème des 5 philosophes
echo "=== Test du problème 5 philosophes ==="
echo "Configuration: 5 600 200 200"
echo "Philosophe attendu à mourir: celui qui n'a pas mangé pendant 600ms"
echo

cd /home/luda-cun/42/Milestone_3/philosopher1

# Compilation
make > /dev/null 2>&1

echo "Exécution du test (timeout 10s)..."
timeout 10s ./philo 5 600 200 200 | tee /tmp/philo_5_test.log

echo
echo "=== Analyse des résultats ==="

# Trouver le moment de la mort
death_line=$(grep "died" /tmp/philo_5_test.log)
if [ -n "$death_line" ]; then
    death_time=$(echo "$death_line" | awk '{print $1}')
    death_philo=$(echo "$death_line" | awk '{print $2}')
    echo "Philosophe $death_philo est mort à ${death_time}ms"
    
    # Vérifier si c'est dans la plage attendue (600ms ± marge)
    if [ "$death_time" -ge 595 ] && [ "$death_time" -le 605 ]; then
        echo "✓ Timing correct (~600ms)"
    else
        echo "✗ Timing incorrect (attendu ~600ms, obtenu ${death_time}ms)"
    fi
else
    echo "✗ Aucune mort détectée - PROBLÈME!"
fi

echo
echo "=== Timeline des repas ==="
grep "is eating" /tmp/philo_5_test.log | head -10

echo
echo "=== Dernier repas de chaque philosophe ==="
for i in {1..5}; do
    last_meal=$(grep "$i is eating" /tmp/philo_5_test.log | tail -1)
    if [ -n "$last_meal" ]; then
        echo "Philo $i: $last_meal"
    else
        echo "Philo $i: N'a jamais mangé!"
    fi
done

rm -f /tmp/philo_5_test.log
